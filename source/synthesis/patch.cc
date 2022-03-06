#include "patch.h"

#include <absl/strings/str_format.h>
#include <glog/logging.h>
#include <pluginterfaces/base/ustring.h>
#include <public.sdk/source/main/pluginfactory_constexpr.h>

#include "constants.h"
#include "tags.h"

namespace sidebands {

using Steinberg::Vst::ParameterInfo;

IPtr<RangeParameter> BooleanParameter(Steinberg::Vst::UnitID unit_id,
                                      const std::string &name, ParamTag tag,
                                      TargetTag sp, uint32_t gen_num) {
  auto info = ParameterInfo{
      .id = TagFor(gen_num, tag, sp),
      .stepCount = 1,
      .defaultNormalizedValue = 0,
      .unitId = unit_id,
  };
  std::string full_name = absl::StrFormat("Gen %d %s", gen_num, name);
  Steinberg::UString(info.title, USTRINGSIZE(info.title))
      .assign(USTRING(full_name.c_str()));

  return new RangeParameter(info, 0, 1);
}

IPtr<RangeParameter> GenericParameter(Steinberg::Vst::UnitID unit_id,
                                      const std::string &name, ParamTag tag,
                                      TargetTag sp, uint32_t gen_num,
                                      ParamValue min, ParamValue max) {
  std::string full_name = absl::StrFormat("Gen %d %s", gen_num, name);

  auto info = ParameterInfo{
      .id = TagFor(gen_num, tag, sp),
      .stepCount = 0,
      .defaultNormalizedValue = 0,
      .unitId = unit_id,
  };
  Steinberg::UString(info.title, USTRINGSIZE(info.title))
      .assign(USTRING(full_name.c_str()));

  return new RangeParameter(info, min, max);
}

IPtr<RangeParameter> ModTypeParameter(Steinberg::Vst::UnitID unit_id,
                                      TargetTag target, uint32_t gen_num) {
  std::string full_name =
      absl::StrFormat("Gen %d Mod Type %s", gen_num, kTargetNames[target]);

  auto info = ParameterInfo{
      .id = TagFor(gen_num, TAG_MOD_TYPE, target),
      .stepCount = 1,
      .defaultNormalizedValue = 0,
      .unitId = unit_id,
  };
  Steinberg::UString(info.title, USTRINGSIZE(info.title))
      .assign(USTRING(full_name.c_str()));

  return new RangeParameter(info, 0, GeneratorPatch::kNumModTypes - 1);
}

IPtr<RangeParameter> OscillatorParameter(Steinberg::Vst::UnitID unit_id,
                                         const std::string &name, TargetTag sp,
                                         uint32_t gen_num, ParamValue min,
                                         ParamValue max) {
  std::string full_name =
      absl::StrFormat("Gen %d Oscillator %s", gen_num, name);

  auto info = ParameterInfo{
      .id = TagFor(gen_num, TAG_OSC, sp),
      .stepCount = 0,
      .defaultNormalizedValue = 0,
      .unitId = unit_id,
  };
  Steinberg::UString(info.title, USTRINGSIZE(info.title))
      .assign(USTRING(full_name.c_str()));

  return new RangeParameter(info, min, max);
}

IPtr<RangeParameter> LFOParameter(Steinberg::Vst::UnitID unit_id,
                                  const std::string &name, ParamTag tag,
                                  TargetTag sp, uint32_t gen_num,
                                  ParamValue min, ParamValue max) {
  std::string full_name = absl::StrFormat("Gen %d LFO %s", gen_num, name);

  auto info = ParameterInfo{
      .id = TagFor(gen_num, tag, sp),
      .stepCount = 0,
      .defaultNormalizedValue = 0,
      .unitId = unit_id,
  };
  Steinberg::UString(info.title, USTRINGSIZE(info.title))
      .assign(USTRING(full_name.c_str()));

  return new RangeParameter(info, min, max);
}

IPtr<RangeParameter> EnvelopeParameter(Steinberg::Vst::UnitID unit_id,
                                       const std::string &name, ParamTag tag,
                                       TargetTag sp, uint32_t gen_num,
                                       ParamValue min, ParamValue max) {
  std::string full_name = absl::StrFormat("Gen %d Envelope %s", gen_num, name);

  auto info = ParameterInfo{
      .id = TagFor(gen_num, tag, sp),
      .stepCount = 0,
      .defaultNormalizedValue = 0,
      .unitId = unit_id,
  };
  Steinberg::UString(info.title, USTRINGSIZE(info.title))
      .assign(USTRING(full_name.c_str()));

  return new RangeParameter(info, min, max);
}

void GeneratorPatch::DeclareEnvelopeParameters(Steinberg::Vst::UnitID unit_id,
                                               TargetTag target,
                                               uint32_t gen_num) {
  std::string env_name = kTargetNames[target];
  DeclareParameter(&envelope_parameters_[target].A_R,
                   EnvelopeParameter(unit_id, env_name + " A", TAG_ENV_A,
                                     target, gen_num, 0, 5));
  DeclareParameter(&envelope_parameters_[target].A_L,
                   EnvelopeParameter(unit_id, env_name + " AL", TAG_ENV_AL,
                                     target, gen_num, 0, 1));
  DeclareParameter(&envelope_parameters_[target].D_R,
                   EnvelopeParameter(unit_id, env_name + " D", TAG_ENV_D,
                                     target, gen_num, 0, 5));
  DeclareParameter(&envelope_parameters_[target].S_L,
                   EnvelopeParameter(unit_id, env_name + " S", TAG_ENV_S,
                                     target, gen_num, 0, 1));
  DeclareParameter(&envelope_parameters_[target].R_R,
                   EnvelopeParameter(unit_id, env_name + " R", TAG_ENV_R,
                                     target, gen_num, 0, 5));
  DeclareParameter(&envelope_parameters_[target].VS,
                   EnvelopeParameter(unit_id, env_name + " VS", TAG_ENV_VS,
                                     target, gen_num, 0, 1));
}

Patch::Patch() {
  for (int g = 0; g < kNumGenerators; g++) {
    auto unit_id = MakeUnitID(UNIT_GENERATOR, g);

    generators_[g] = std::make_unique<GeneratorPatch>(g, unit_id);
  }
}

void Patch::AppendParameters(ParameterContainer *container) {
  for (auto &generator : generators_) {
    generator->AppendParameters(container);
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

bool Patch::ValidParam(ParamID param_id) const {
  return GeneratorFor(param_id) < kNumGenerators;
}

GeneratorPatch::GeneratorPatch(uint32_t gen, Steinberg::Vst::UnitID unit_id)
    : gennum_(gen), on_(nullptr),
      c_(TagFor(gennum_, TAG_OSC, TARGET_C), 1 + gennum_),
      a_(TagFor(gennum_, TAG_OSC, TARGET_A), 0.5),
      m_(TagFor(gennum_, TAG_OSC, TARGET_M), 0),
      k_(TagFor(gennum_, TAG_OSC, TARGET_K), 0),
      r_(TagFor(gennum_, TAG_OSC, TARGET_R), 1),
      s_(TagFor(gennum_, TAG_OSC, TARGET_S), 0) {

  on_ = DeclareParameter(BooleanParameter(
      unit_id, "Toggle", TAG_GENERATOR_TOGGLE, TARGET_NA, gennum_));

  DeclareParameter(&c_, OscillatorParameter(unit_id, "C", TARGET_C, gennum_, 0,
                                            kNumGenerators));
  DeclareParameter(&a_,
                   OscillatorParameter(unit_id, "A", TARGET_A, gennum_, 0, 1));
  DeclareParameter(&m_,
                   OscillatorParameter(unit_id, "M", TARGET_M, gennum_, 0, 8));
  DeclareParameter(&k_,
                   OscillatorParameter(unit_id, "K", TARGET_K, gennum_, 0, 10));
  DeclareParameter(&r_,
                   OscillatorParameter(unit_id, "R", TARGET_R, gennum_, 0, 1));
  DeclareParameter(&s_,
                   OscillatorParameter(unit_id, "S", TARGET_S, gennum_, -1, 1));
  DeclareParameter(
      &portamento_,
      OscillatorParameter(unit_id, "Port", TARGET_PORTAMENTO, gennum_, 0, 1));

  for (auto &target : kModulationTargets) {
    DeclareParameter(&mod_type_[target], DeclareParameter(ModTypeParameter(
                                             unit_id, target, gennum_)));
    DeclareEnvelopeParameters(unit_id, target, gennum_);
    lfo_parameters_[target].amplitude_ = DeclareParameter(
        LFOParameter(unit_id, "Amp", TAG_LFO_AMP, target, gennum_, 0, 1.0));
    lfo_parameters_[target].frequency_ = DeclareParameter(
        LFOParameter(unit_id, "Freq", TAG_LFO_FREQ, target, gennum_, 0, 20));
    lfo_parameters_[target].function_ = DeclareParameter(LFOParameter(
        unit_id, "Func", TAG_LFO_TYPE, target, gennum_, 0, kNumLFOTypes - 1));
    lfo_parameters_[target].vel_sense_ = DeclareParameter(LFOParameter(
        unit_id, "VelSens", TAG_LFO_VS, target, gennum_, 0, kNumLFOTypes - 1));
  }
}

IPtr<RangeParameter> GeneratorPatch::DeclareParameter(
    SampleAccurateValue *value,
    IPtr<RangeParameter> param) {

  value->setParamID(param->getInfo().id);
  value->minPlain = param->getMin();
  value->maxPlain = param->getMax();

  auto param_key = ParamKeyFor(param->getInfo().id);

  std::lock_guard<std::mutex> params_lock(patch_mutex_);
  parameters_[param_key] = std::move(
      std::make_unique<PDesc>(value, PDesc::Type::SAMPLE_ACCURATE, param));

  return param;
}

IPtr<RangeParameter>
GeneratorPatch::DeclareParameter(IPtr<RangeParameter> param) {
  std::lock_guard<std::mutex> params_lock(patch_mutex_);

  auto param_key = ParamKeyFor(param->getInfo().id);
  parameters_[param_key] =
      std::move(std::make_unique<PDesc>(nullptr, PDesc::Type::VALUE, param));

  return param;
}

void GeneratorPatch::AppendParameters(ParameterContainer *container) {
  std::lock_guard<std::mutex> params_lock(patch_mutex_);

  for (auto &pdesc : parameters_) {
    if (pdesc.second) {
      auto *param = container->addParameter(pdesc.second->param);
      param->addDependent(this);
      param->setUnitID(MakeUnitID(UNIT_GENERATOR, gennum_));
    }
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
  switch (param->type) {
  case PDesc::Type::SAMPLE_ACCURATE:
    param->sa_value->beginChanges(p_queue);
    break;
  }
}

void GeneratorPatch::EndChanges() {
  std::lock_guard<std::mutex> params_lock(patch_mutex_);
  for (auto &pdesc : parameters_) {
    if (pdesc.second && pdesc.second->type == PDesc::Type::SAMPLE_ACCURATE) {
      pdesc.second->sa_value->endChanges();
    }
  }
}

void GeneratorPatch::AdvanceParameterChanges(uint32_t num_samples) {
  std::lock_guard<std::mutex> params_lock(patch_mutex_);
  for (auto &pdesc : parameters_) {
    if (pdesc.second && pdesc.second->type == PDesc::Type::SAMPLE_ACCURATE) {
      bool has_changes = pdesc.second->sa_value->hasChanges();
      auto old = pdesc.second->sa_value->getValue();
      pdesc.second->sa_value->advance(num_samples);
      if (has_changes) {
        LOG(INFO) << "Advanced " << TagStr(pdesc.second->param->getInfo().id)
                  << " to " << pdesc.second->sa_value->getValue() << " from " << old;
      }
    }
  }
}

// Reflect changes in the parameter object to inside the sample accurate
// internal rep of it.
void GeneratorPatch::update(FUnknown *changedUnknown,
                            Steinberg::int32 message) {
  if (message == IDependent::kChanged) {
    Steinberg::Vst::RangeParameter *changed_param;
    changedUnknown->queryInterface(Parameter::iid, (void **)&changed_param);
    std::lock_guard<std::mutex> params_lock(patch_mutex_);

    for (auto &param : parameters_) {
      if (param.second && param.second->param &&
          param.second->param->getInfo().id == changed_param->getInfo().id) {
        switch (param.second->type) {
        case PDesc::Type::SAMPLE_ACCURATE:
          ParamValue v = changed_param->toPlain(changed_param->getNormalized());
          LOG(INFO) << "Changed: " << TagStr(changed_param->getInfo().id)
                    << " to " << v << "(" << changed_param->getNormalized()
                    << ")";
          param.second->sa_value->setValue(v);
          break;
        }
      }
    }
  }
}

bool GeneratorPatch::on() const {
  std::lock_guard<std::mutex> params_lock(patch_mutex_);
  return on_->getNormalized();
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
    auto &target_env = envelope_parameters_[destination];
    return target_env;
  }

  if (ModTypeFor(destination) == ModType::LFO) {
    auto &target_lfo = lfo_parameters_[destination];
    return LFOValues{
        .type = kLFOTypes[static_cast<int>(
            target_lfo.function_->getNormalized() * kNumLFOTypes)],
        .frequency = target_lfo.frequency_->toPlain(
            target_lfo.frequency_->getNormalized()),
        .amplitude = target_lfo.amplitude_->toPlain(
            target_lfo.amplitude_->getNormalized()),
        .velocity_sensitivty = target_lfo.vel_sense_->toPlain(
            target_lfo.vel_sense_->getNormalized()),
    };
  }

  return std::nullopt;
}

GeneratorPatch::ModType
GeneratorPatch::ModTypeFor(TargetTag destination) const {
  return kModTypes[off_t(mod_type_[destination].getValue())];
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