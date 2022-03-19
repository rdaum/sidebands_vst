#include "patch_processor.h"

#include <absl/strings/str_format.h>
#include <base/source/fstreamer.h>
#include <glog/logging.h>
#include <pluginterfaces/base/ustring.h>

#include "constants.h"
#include "tags.h"

using Steinberg::Vst::ParamID;
using Steinberg::Vst::ParamValue;

namespace sidebands {

namespace {

void WriteParameter(Steinberg::IBStreamer &streamer, int gennum, ParamTag param,
                    TargetTag target, ParamValue value) {
  // Format is tag followed by value.
  // Everything is float, at least for now, because our parameter framework
  // doesn't really know anything else.
  streamer.writeInt32u(TagFor(gennum, param, target));
  streamer.writeDouble(value);
}

} // namespace

// static
bool PatchProcessor::ValidParam(ParamID param_id) {
  return GeneratorFor(param_id) < kNumGenerators;
}

PatchProcessor::PatchProcessor() {
  for (int g = 0; g < kNumGenerators; g++) {
    auto unit_id = MakeUnitID(UNIT_GENERATOR, g);

    generators_[g] = std::make_unique<GeneratorPatch>(g, unit_id);
  }
}

void PatchProcessor::BeginParameterChange(
    ParamID param_id, Steinberg::Vst::IParamValueQueue *p_queue) {
  uint8_t gen_num = GeneratorFor(param_id);
  generators_[gen_num]->BeginParameterChange(param_id, p_queue);
}

void PatchProcessor::EndParameterChanges() {
  for (auto &generator : generators_) {
    generator->EndChanges();
  }
}

void PatchProcessor::AdvanceParameterChanges(uint32_t num_samples) {
  for (auto &item : generators_) {
    item->AdvanceParameterChanges(num_samples);
  }
}

Steinberg::tresult PatchProcessor::LoadPatch(Steinberg::IBStream *stream) {
  // called when we load a preset, the model has to be reloaded
  Steinberg::IBStreamer streamer(stream, kLittleEndian);

  Steinberg::uint32 num_generators;
  if (!streamer.readInt32u(num_generators)) {
    LOG(ERROR) << "Could not read patch stream; expected generator count.";
    return Steinberg::kResultFalse;
  }
  if (num_generators != kNumGenerators) {
    LOG(ERROR) << "Incompatible generator count. Got: " << num_generators
               << " expected: " << kNumGenerators;
    return Steinberg::kResultFalse;
  }
  for (int i = 0; i < kNumGenerators; i++) {
    generators_[i]->LoadPatch(streamer);
  }

  return Steinberg::kResultOk;
}

Steinberg::tresult PatchProcessor::SavePatch(Steinberg::IBStream *stream) {
  // called when we load a preset, the model has to be reloaded
  Steinberg::IBStreamer streamer(stream, kLittleEndian);

  streamer.writeInt32u(kNumGenerators);
  for (int i = 0; i < kNumGenerators; i++) {
    generators_[i]->SavePatch(streamer);
  }

  return Steinberg::kResultOk;
}

Steinberg::tresult GeneratorPatch::LoadPatch(Steinberg::IBStreamer &streamer) {
  Steinberg::uint32 stream_gennum, num_params;
  if (!streamer.readInt32u(stream_gennum)) {
    LOG(ERROR) << "Unable to read start of generator: " << gennum_;
    return Steinberg::kResultFalse;
  }
  if (stream_gennum != gennum_) {
    LOG(ERROR) << "Wrong generator # in file; was: " << stream_gennum
               << " expected: " << gennum_;
    return Steinberg::kResultFalse;
  }
  if (!streamer.readInt32u(num_params)) {
    LOG(ERROR) << "Unable to read parameter count for generator: " << gennum_;
    return Steinberg::kResultFalse;
  }
  while (num_params--) {
    Steinberg::Vst::ParamID id;
    if (!streamer.readInt32u(id))
      break;
    ParamValue v;
    CHECK(streamer.readDouble(v))
        << "Unable to read value for param id: " << TagStr(id);
    ParamKey k{ParamFor(id), TargetFor(id)};
    const auto &it = parameters_.find(k);
    if (it == parameters_.end()) {
      LOG(ERROR) << " Missing parameter for id: " << TagStr(id);
      continue;
    }
    auto plain = v * (it->second.min + (it->second.max - it->second.min));
    if (it->second.sa) {
      it->second.v.sv->setValueNormalized(v);
    } else {
      *it->second.v.v = plain;
    }
  }
  return Steinberg::kResultOk;
}

Steinberg::tresult GeneratorPatch::SavePatch(Steinberg::IBStreamer &streamer) {

  // First write generator number, then the # of parameters.
  streamer.writeInt32u(gennum_);
  streamer.writeInt32u(parameters_.size());
  for (auto &param : parameters_) {
    // Format is tag followed by value.
    // Everything is float, at least for now, because our parameter framework
    // doesn't really know anything else.

    Param &p = param.second;
    ParamValue v = p.sa ? p.v.sv->getValue() : *p.v.v;
    auto nv = (v - p.min) / (p.max - p.min);
    WriteParameter(streamer, gennum_, param.first.p_tag, param.first.target,
                   nv);
  }
  return Steinberg::kResultOk;
}

void GeneratorPatch::DeclareParameter(SampleAccurateValue *value) {
  auto param_key = ParamKeyFor(value->getParamID());

  std::lock_guard<std::mutex> params_lock(patch_mutex_);

  parameters_[param_key] = {{.sv = value}, true, value->min(), value->max()};
}

void GeneratorPatch::DeclareParameter(ParamID param_id,
                                      Steinberg::Vst::ParamValue *value,
                                      ParamValue min, ParamValue max) {
  std::lock_guard<std::mutex> params_lock(patch_mutex_);
  auto param_key = ParamKeyFor(param_id);

  parameters_[param_key].sa = false;
  parameters_[param_key].v.v = value;
  parameters_[param_key].min = min;
  parameters_[param_key].max = max;
}

GeneratorPatch::GeneratorPatch(uint32_t gen, Steinberg::Vst::UnitID unit_id)
    : gennum_(gen), on_(gen == 0),
      c_(TagFor(gennum_, TAG_OSC, TARGET_C), 1 + gennum_, 0, 8),
      a_(TagFor(gennum_, TAG_OSC, TARGET_A), 0.5, 0, 1),
      m_(TagFor(gennum_, TAG_OSC, TARGET_M), 4, 0, 8),
      k_(TagFor(gennum_, TAG_OSC, TARGET_K), 1, 0, 10),
      r_(TagFor(gennum_, TAG_OSC, TARGET_R), 1, 0, 1),
      s_(TagFor(gennum_, TAG_OSC, TARGET_S), 0, -1, 1),
      portamento_(TagFor(gennum_, TAG_OSC, TARGET_PORTAMENTO), 0, 0, 1) {

  DeclareParameter(TagFor(gen, TAG_GENERATOR_TOGGLE, TARGET_NA), &on_, 0, 1);
  DeclareParameter(&c_);
  DeclareParameter(&a_);
  DeclareParameter(&m_);
  DeclareParameter(&k_);
  DeclareParameter(&r_);
  DeclareParameter(&s_);
  DeclareParameter(&portamento_);

  for (auto &target : kModulationTargets) {
    auto default_mod_type = target == TARGET_A || target == TARGET_K
                                ? (int)ModType::ADSR_ENVELOPE
                                : (int)ModType::NONE;

    ParamValue mod_type(default_mod_type);
    ADSREnvelopeValues envelope_values{
        SampleAccurateValue(TagFor(gennum_, TAG_ENV_HT, target), 0.0, 0, 1),
        SampleAccurateValue(TagFor(gennum_, TAG_ENV_AR, target), 0.67, 0, 1),
        SampleAccurateValue(TagFor(gennum_, TAG_ENV_AL, target), 0.9, 0, 1),
        SampleAccurateValue(TagFor(gennum_, TAG_ENV_DR1, target), 0.50, 0, 1),
        SampleAccurateValue(TagFor(gennum_, TAG_ENV_DL1, target), 0.9, 0, 1),
        SampleAccurateValue(TagFor(gennum_, TAG_ENV_DR2, target), 0.10, 0, 1),
        SampleAccurateValue(TagFor(gennum_, TAG_ENV_SL, target), 0.20, 0, 1),
        SampleAccurateValue(TagFor(gennum_, TAG_ENV_RR1, target), 0.10, 0, 1),
        SampleAccurateValue(TagFor(gennum_, TAG_ENV_RL1, target), 0.10, 0, 1),
        SampleAccurateValue(TagFor(gennum_, TAG_ENV_RR2, target), 0.10, 0, 1),
        SampleAccurateValue(TagFor(gennum_, TAG_ENV_VS, target), 1, 0, 1),
    };
    LFOValues lfo_values{
        Steinberg::Vst::ParamValue(LFOType::SIN),
        SampleAccurateValue(TagFor(gennum_, TAG_LFO_FREQ, target), 10, 0, 20),
        SampleAccurateValue(TagFor(gennum_, TAG_LFO_AMP, target), 0.5, 0, 1),
        SampleAccurateValue(TagFor(gennum_, TAG_LFO_VS, target), 1, 0, 1),
    };
    auto mod_target = std::make_unique<ModParams>(target, mod_type,
                                                  envelope_values, lfo_values);
    mod_targets_[target] = std::move(mod_target);
  };

  // Take pointers _after_ the vector is fully constructed, just in case things
  // get moved.
  for (auto &mt : mod_targets_) {
    if (!mt)
      continue;
    DeclareParameter(TagFor(gennum_, TAG_MOD_TYPE, mt->target), &mt->mod_type,
                     0, kNumModTypes - 1);
    DeclareParameter(&mt->adsr_parameters.HT);
    DeclareParameter(&mt->adsr_parameters.AL);
    DeclareParameter(&mt->adsr_parameters.AR);
    DeclareParameter(&mt->adsr_parameters.DL1);
    DeclareParameter(&mt->adsr_parameters.DR1);
    DeclareParameter(&mt->adsr_parameters.DR2);
    DeclareParameter(&mt->adsr_parameters.SL);
    DeclareParameter(&mt->adsr_parameters.RR1);
    DeclareParameter(&mt->adsr_parameters.RR2);
    DeclareParameter(&mt->adsr_parameters.VS);
    DeclareParameter(TagFor(gennum_, TAG_LFO_TYPE, mt->target),
                     &mt->lfo_parameters.type, 0, kNumLFOTypes - 1);
    DeclareParameter(&mt->lfo_parameters.amplitude);
    DeclareParameter(&mt->lfo_parameters.frequency);
    DeclareParameter(&mt->lfo_parameters.velocity_sensivity);
  }
}

std::optional<ParamValue>
GetLastValue(Steinberg::Vst::IParamValueQueue *p_queue) {
  ParamValue value;
  int32_t numPoints = p_queue->getPointCount();
  Steinberg::int32 sample_offset;
  if (p_queue->getPoint(numPoints - 1, sample_offset, value) ==
      Steinberg::kResultTrue) {
    return value;
  }
  return std::nullopt;
}

void GeneratorPatch::BeginParameterChange(
    ParamID param_id, Steinberg::Vst::IParamValueQueue *p_queue) {
  if (!p_queue->getPointCount())
    return;

  std::lock_guard<std::mutex> params_lock(patch_mutex_);
  if (GeneratorFor(param_id) != gennum_)
    return;

  auto key = ParamKeyFor(param_id);
  const auto &param_it = parameters_.find(key);
  if (param_it == parameters_.end())
    return;
  auto param = param_it->second;
  if (param.sa) {
    param.v.sv->beginChanges(p_queue);
    sa_changed_params_.push_back(param_id);
  } else {
    const auto &last_v_opt = GetLastValue(p_queue);
    if (last_v_opt) {
      auto nv = param.min + (last_v_opt.value() * (param.max - param.min));
      LOG(INFO) << "Setting non-Sample accurate parameter: " << TagStr(param_id)
                << " to  " << nv << " (was: " << *param.v.v << ")";

      *param.v.v = nv;
    }
  }
}

void GeneratorPatch::EndChanges() {
  std::lock_guard<std::mutex> params_lock(patch_mutex_);
  while (!sa_changed_params_.empty()) {
    auto changed_param = sa_changed_params_.front();
    auto key = ParamKeyFor(changed_param);
    auto pdesc = parameters_[key];
    sa_changed_params_.pop_back();
    if (pdesc.sa) {
      pdesc.v.sv->endChanges();
    }
  }
}

void GeneratorPatch::AdvanceParameterChanges(uint32_t num_samples) {
  std::lock_guard<std::mutex> params_lock(patch_mutex_);
  for (auto &pdesc : parameters_) {
    if (pdesc.second.sa) {
      pdesc.second.v.sv->advance(num_samples);
    }
  }
}

bool GeneratorPatch::on() const {
  std::lock_guard<std::mutex> params_lock(patch_mutex_);
  return on_;
}

ParamValue GeneratorPatch::c() const {
  std::lock_guard<std::mutex> params_lock(patch_mutex_);
  return c_.getValue();
}

ParamValue GeneratorPatch::a() const {
  std::lock_guard<std::mutex> params_lock(patch_mutex_);
  return a_.getValue();
}

ParamValue GeneratorPatch::m() const {
  std::lock_guard<std::mutex> params_lock(patch_mutex_);
  return m_.getValue();
}

ParamValue GeneratorPatch::k() const {
  std::lock_guard<std::mutex> params_lock(patch_mutex_);
  return k_.getValue();
}

ParamValue GeneratorPatch::r() const {
  std::lock_guard<std::mutex> params_lock(patch_mutex_);
  return r_.getValue();
}

ParamValue GeneratorPatch::s() const {
  std::lock_guard<std::mutex> params_lock(patch_mutex_);
  return s_.getValue();
}

ParamValue GeneratorPatch::portamento() const {
  std::lock_guard<std::mutex> params_lock(patch_mutex_);
  return portamento_.getValue();
}

GeneratorPatch::ModParams *
GeneratorPatch::ModulationParams(TargetTag destination) const {
  return mod_targets_[destination].get();
}

ModType GeneratorPatch::ModTypeFor(TargetTag destination) const {
  auto &mod_target = mod_targets_[destination];
  if (!mod_target)
    return ModType::NONE;
  auto index = off_t(mod_target->mod_type);
  ModType mod_type = kModTypes[index];
  return mod_type;
}

std::function<double()>
GeneratorPatch::ParameterGetterFor(TargetTag dest) const {
  switch (dest) {
  case TARGET_A:
    return std::bind(&GeneratorPatch::a, this);
  case TARGET_K:
    return std::bind(&GeneratorPatch::k, this);
  case TARGET_C:
    return std::bind(&GeneratorPatch::c, this);
  case TARGET_M:
    return std::bind(&GeneratorPatch::m, this);
  case TARGET_R:
    return std::bind(&GeneratorPatch::r, this);
  case TARGET_S:
    return std::bind(&GeneratorPatch::s, this);
  default:
    LOG(ERROR) << "Unknown parameter: " << dest;
    assert(0);
    return std::bind(&GeneratorPatch::a, this);
  }
}

} // namespace sidebands