#include "patch.h"

#include <absl/strings/str_format.h>
#include <glog/logging.h>
#include <pluginterfaces/base/ustring.h>
#include <public.sdk/source/main/pluginfactory_constexpr.h>

#include "constants.h"
#include "tags.h"

namespace sidebands {

using Steinberg::Vst::ParameterInfo;
using SAP = Steinberg::Vst::SampleAccurate::Parameter;

IPtr<RangeParameter> BooleanParameter(const std::string &name, ParamTag tag,
                                      TargetTag sp, uint32_t gen_num) {
  std::string full_name = absl::StrFormat("Gen %d %s", gen_num, name);

  auto info = ParameterInfo{
      .id = TagFor(gen_num, tag, sp),
      .stepCount = 1,
      .defaultNormalizedValue = 0,
  };
  Steinberg::UString(info.title, USTRINGSIZE(info.title))
      .assign(USTRING(full_name.c_str()));

  return new RangeParameter(info, 0, 1);
}

IPtr<RangeParameter> ModTypeParameter(TargetTag target, uint32_t gen_num) {
  std::string full_name =
      absl::StrFormat("Gen %d Mod Type %s", gen_num, kTargetNames[target]);

  auto info = ParameterInfo{
      .id = TagFor(gen_num, TAG_MOD_TYPE, target),
      .stepCount = 1,
      .defaultNormalizedValue = 0,
  };
  Steinberg::UString(info.title, USTRINGSIZE(info.title))
      .assign(USTRING(full_name.c_str()));

  return new RangeParameter(info, 0, GeneratorPatch::kNumModTypes);
}

IPtr<RangeParameter> OscillatorParameter(const std::string &name, TargetTag sp,
                                         uint32_t gen_num, ParamValue min,
                                         ParamValue max) {
  std::string full_name =
      absl::StrFormat("Gen %d Oscillator %s", gen_num, name);

  auto info = ParameterInfo{
      .id = TagFor(gen_num, TAG_OSC, sp),
      .stepCount = 0,
      .defaultNormalizedValue = 0,
  };
  Steinberg::UString(info.title, USTRINGSIZE(info.title))
      .assign(USTRING(full_name.c_str()));

  return new RangeParameter(info, min, max);
}

IPtr<RangeParameter> LFOParameter(const std::string &name, ParamTag tag,
                                  TargetTag sp, uint32_t gen_num,
                                  ParamValue min, ParamValue max) {
  std::string full_name = absl::StrFormat("Gen %d LFO %s", gen_num, name);

  auto info = ParameterInfo{
      .id = TagFor(gen_num, tag, sp),
      .stepCount = 0,
      .defaultNormalizedValue = 0,
  };
  Steinberg::UString(info.title, USTRINGSIZE(info.title))
      .assign(USTRING(full_name.c_str()));

  return new RangeParameter(info, min, max);
}

IPtr<RangeParameter> EnvelopeParameter(const std::string &name, ParamTag tag,
                                       TargetTag sp, uint32_t gen_num,
                                       ParamValue min, ParamValue max) {
  std::string full_name = absl::StrFormat("Gen %d Envelope %s", gen_num, name);

  auto info = ParameterInfo{
      .id = TagFor(gen_num, tag, sp),
      .stepCount = 0,
      .defaultNormalizedValue = 0,
  };
  Steinberg::UString(info.title, USTRINGSIZE(info.title))
      .assign(USTRING(full_name.c_str()));

  return new RangeParameter(info, min, max);
}

void GeneratorPatch::DeclareEnvelopeParameters(TargetTag target,
                                               uint32_t gen_num) {
  std::string env_name = kTargetNames[target];
  GeneratorPatch::EnvelopeParameters params;
  params.a_r_ = DeclareParameter(
      EnvelopeParameter(env_name + " A", TAG_ENV_A, target, gen_num, 0, 5));
  params.a_l_ = DeclareParameter(
      EnvelopeParameter(env_name + " AL", TAG_ENV_AL, target, gen_num, 0, 1));
  params.d_r_ = DeclareParameter(
      EnvelopeParameter(env_name + " D", TAG_ENV_D, target, gen_num, 0, 5));
  params.s_l_ = DeclareParameter(
      EnvelopeParameter(env_name + " S", TAG_ENV_S, target, gen_num, 0, 1));
  params.r_r_ = DeclareParameter(
      EnvelopeParameter(env_name + " R", TAG_ENV_R, target, gen_num, 0, 5));

  envelope_parameters_[target] = std::move(params);
}

Patch::Patch() {
  for (int g = 0; g < kNumGenerators; g++) {
    generators[g] = std::make_unique<GeneratorPatch>(g);
  }
}

void Patch::AppendParameters(ParameterContainer *container) {
  for (auto &generator : generators) {
    generator->AppendParameters(container);
  }
}

void Patch::BeginParameterChange(ParamID param_id,
                                 Steinberg::Vst::IParamValueQueue *p_queue) {
  generators[GeneratorFor(param_id)]->BeginParameterChange(param_id, p_queue);
}

void Patch::EndParameterChanges() {
  for (auto &item : generators) {
    item->EndChanges();
  }
}

void Patch::AdvanceParameterChanges(uint32_t num_samples) {
  for (auto &item : generators) {
    item->AdvanceParameterChanges(num_samples);
  }
}

GeneratorPatch::GeneratorPatch(uint32_t gen)
    : gennum_(gen), on_(nullptr),
      c_(TagFor(gennum_, TAG_OSC, TARGET_C), 1 + gennum_),
      a_(TagFor(gennum_, TAG_OSC, TARGET_A), 0.5),
      m_(TagFor(gennum_, TAG_OSC, TARGET_M), 0),
      k_(TagFor(gennum_, TAG_OSC, TARGET_K), 0),
      r_(TagFor(gennum_, TAG_OSC, TARGET_R), 1),
      s_(TagFor(gennum_, TAG_OSC, TARGET_S), 0) {

  on_ = DeclareParameter(
      BooleanParameter("Toggle", TAG_GENERATOR_TOGGLE, TARGET_NA, gennum_));

  DeclareParameter(
      &c_, OscillatorParameter("C", TARGET_C, gennum_, 0, kNumGenerators));
  DeclareParameter(&a_, OscillatorParameter("A", TARGET_A, gennum_, 0, 1));
  DeclareParameter(&m_, OscillatorParameter("M", TARGET_M, gennum_, 0, 8));
  DeclareParameter(&k_, OscillatorParameter("K", TARGET_K, gennum_, 0, 8));
  DeclareParameter(&r_, OscillatorParameter("R", TARGET_R, gennum_, 0, 1));
  DeclareParameter(&s_, OscillatorParameter("S", TARGET_S, gennum_, -1, 1));

  for (auto &target : kModulationTargets) {
    mod_type_[target] = DeclareParameter(ModTypeParameter(target, gennum_));
    DeclareEnvelopeParameters(target, gennum_);
  }
}

IPtr<RangeParameter> GeneratorPatch::DeclareParameter(
    Steinberg::Vst::SampleAccurate::Parameter *param_v,
    IPtr<RangeParameter> param) {
  std::lock_guard<std::mutex> params_lock(patch_mutex_);

  auto param_key = ParamKeyFor(param->getInfo().id);
  parameters_[param_key] = std::move(
      std::make_unique<PDesc>(param_v, PDesc::Type::SAMPLE_ACCURATE, param));

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
  std::lock_guard<std::mutex> params_lock(patch_mutex_);
  if (GeneratorFor(param_id) != gennum_)
    return;

  auto key = ParamKeyFor(param_id);
  auto &param = parameters_[key];
  if (!param)
    return;
  switch (param->type) {
  case PDesc::Type::SAMPLE_ACCURATE:
    param->sa_param->beginChanges(p_queue);
    break;
  }
}

void GeneratorPatch::EndChanges() {
  std::lock_guard<std::mutex> params_lock(patch_mutex_);
  for (auto &pdesc : parameters_) {
    if (pdesc.second && pdesc.second->type == PDesc::Type::SAMPLE_ACCURATE)
      pdesc.second->sa_param->endChanges();
  }
}

void GeneratorPatch::AdvanceParameterChanges(uint32_t num_samples) {
  std::lock_guard<std::mutex> params_lock(patch_mutex_);
  for (auto &pdesc : parameters_) {
    if (pdesc.second && pdesc.second->type == PDesc::Type::SAMPLE_ACCURATE) {
      pdesc.second->sa_param->advance(num_samples);
    }
  }
}

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
          param.second->sa_param->setValue(
              changed_param->toPlain(changed_param->getNormalized()));
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

std::optional<GeneratorPatch::ModulationParameters>
GeneratorPatch::ModulationParams(TargetTag destination) const {
  auto &target_env = envelope_parameters_[destination];
  return EnvelopeValues{
      .A_R = target_env.a_r_->toPlain(target_env.a_r_->getNormalized()),
      .A_L = target_env.a_l_->toPlain(target_env.a_l_->getNormalized()),
      .D_R = target_env.d_r_->toPlain(target_env.d_r_->getNormalized()),
      .S_L = target_env.s_l_->toPlain(target_env.s_l_->getNormalized()),
      .R_R = target_env.r_r_->toPlain(target_env.r_r_->getNormalized()),
  };
}

GeneratorPatch::ModType
GeneratorPatch::ModTypeFor(TargetTag destination) const {
  IPtr<RangeParameter> mod_type_param = mod_type_[destination];
  int mod_type_val = mod_type_param->toPlain(mod_type_param->getNormalized());
  return kModTypes[mod_type_val];
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