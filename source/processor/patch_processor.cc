#include "patch_processor.h"

#include <absl/strings/str_format.h>
#include <glog/logging.h>
#include <pluginterfaces/base/ustring.h>

#include "constants.h"
#include "tags.h"

namespace sidebands {

// static
bool Patch::ValidParam(ParamID param_id) {
  return GeneratorFor(param_id) < kNumGenerators;
}

Patch::Patch() {
  for (int g = 0; g < kNumGenerators; g++) {
    auto unit_id = MakeUnitID(UNIT_GENERATOR, g);

    generators_[g] = std::make_unique<GeneratorPatch>(g, unit_id);
  }
}

void Patch::BeginParameterChange(ParamID param_id,
                                 Steinberg::Vst::IParamValueQueue *p_queue) {
  uint8_t gen_num = GeneratorFor(param_id);
  generators_[gen_num]->BeginParameterChange(param_id, p_queue);
}

void Patch::EndParameterChanges() {
  for (auto &generator : generators_) {
    generator->EndChanges();
  }
}

void Patch::AdvanceParameterChanges(uint32_t num_samples) {
  for (auto &item : generators_) {
    item->AdvanceParameterChanges(num_samples);
  }
}

void GeneratorPatch::DeclareParameter(SampleAccurateValue *value) {
  auto param_key = ParamKeyFor(value->getParamID());

  std::lock_guard<std::mutex> params_lock(patch_mutex_);
  parameters_[param_key] = value;
}

GeneratorPatch::GeneratorPatch(uint32_t gen, Steinberg::Vst::UnitID unit_id)
    : gennum_(gen),
      on_(TagFor(gennum_, TAG_GENERATOR_TOGGLE, TARGET_NA), false, 0, 1),
      c_(TagFor(gennum_, TAG_OSC, TARGET_C), 1 + gennum_, 0, 8),
      a_(TagFor(gennum_, TAG_OSC, TARGET_A), 0.5, 0, 1),
      m_(TagFor(gennum_, TAG_OSC, TARGET_M), 0, 0, 8),
      k_(TagFor(gennum_, TAG_OSC, TARGET_K), 0, 0, 10),
      r_(TagFor(gennum_, TAG_OSC, TARGET_R), 1, 0, 1),
      s_(TagFor(gennum_, TAG_OSC, TARGET_S), 0, -1, 1),
      portamento_(TagFor(gennum_, TAG_OSC, TARGET_S), 0, 0, 1) {

  DeclareParameter(&on_);
  DeclareParameter(&c_);
  DeclareParameter(&a_);
  DeclareParameter(&m_);
  DeclareParameter(&k_);
  DeclareParameter(&r_);
  DeclareParameter(&s_);
  DeclareParameter(&portamento_);

  for (auto &target : kModulationTargets) {
    SampleAccurateValue mod_type(TagFor(gennum_, TAG_MOD_TYPE, target),
                                 (int)ModType::ENVELOPE, 0, kNumModTypes - 1);
    EnvelopeValues envelope_values{
        SampleAccurateValue(TagFor(gennum_, TAG_ENV_A, target), 0.1, 0, 5),
        SampleAccurateValue(TagFor(gennum_, TAG_ENV_AL, target), 1, 0, 1),
        SampleAccurateValue(TagFor(gennum_, TAG_ENV_D, target), 0.1, 0, 5),
        SampleAccurateValue(TagFor(gennum_, TAG_ENV_S, target), 0.2, 0, 1),
        SampleAccurateValue(TagFor(gennum_, TAG_ENV_R, target), 0.2, 0, 5),
        SampleAccurateValue(TagFor(gennum_, TAG_ENV_VS, target), 1, 0, 1),
    };
    LFOValues lfo_values{
        SampleAccurateValue(TagFor(gennum_, TAG_LFO_TYPE, target),
                            (int)LFOType::SIN, 0, kNumLFOTypes - 1),
        SampleAccurateValue(TagFor(gennum_, TAG_LFO_FREQ, target), 10, 0, 20),
        SampleAccurateValue(TagFor(gennum_, TAG_LFO_AMP, target), 0.5, 0, 1),
        SampleAccurateValue(TagFor(gennum_, TAG_LFO_VS, target), 1, 0, 1),
    };
    ModTarget mod_target = {mod_type, envelope_values, lfo_values};
    mod_targets_.emplace_back(mod_target);
  };

  // Take pointers _after_ the vector is fully constructed, just in case things
  // get moved.
  for (auto &mt : mod_targets_) {
    DeclareParameter(&mt.mod_type);
    DeclareParameter(&mt.lfo_parameters.type);
    DeclareParameter(&mt.lfo_parameters.amplitude);
    DeclareParameter(&mt.lfo_parameters.velocity_sensivity);
    DeclareParameter(&mt.envelope_parameters.A_R);
    DeclareParameter(&mt.envelope_parameters.A_L);
    DeclareParameter(&mt.envelope_parameters.D_R);
    DeclareParameter(&mt.envelope_parameters.S_L);
    DeclareParameter(&mt.envelope_parameters.R_R);
    DeclareParameter(&mt.envelope_parameters.VS);
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
  auto &param = parameters_[key];
  if (!param)
    return;
  param->beginChanges(p_queue);
}

void GeneratorPatch::EndChanges() {
  std::lock_guard<std::mutex> params_lock(patch_mutex_);
  for (auto &pdesc : parameters_) {
    if (pdesc.second)
      pdesc.second->endChanges();
  }
}

void GeneratorPatch::AdvanceParameterChanges(uint32_t num_samples) {
  std::lock_guard<std::mutex> params_lock(patch_mutex_);
  for (auto &pdesc : parameters_) {
    if (pdesc.second)
      pdesc.second->advance(num_samples);
    else
      LOG(ERROR) << "Missing SampleAccurateValue for: "
                 << kParamNames[pdesc.first.p_tag] << "/"
                 << kTargetNames[pdesc.first.target];
  }
}

bool GeneratorPatch::on() const {
  std::lock_guard<std::mutex> params_lock(patch_mutex_);
  return on_.getValue();
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

std::optional<GeneratorPatch::ModulationParameters>
GeneratorPatch::ModulationParams(TargetTag destination) const {
  if (ModTypeFor(destination) == ModType::ENVELOPE) {
    return mod_targets_[destination].envelope_parameters;
  }

  if (ModTypeFor(destination) == ModType::LFO) {
    return mod_targets_[destination].lfo_parameters;
  }

  return std::nullopt;
}

ModType GeneratorPatch::ModTypeFor(TargetTag destination) const {
  return kModTypes[off_t(mod_targets_[destination].mod_type.getValue())];
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