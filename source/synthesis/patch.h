#pragma once

#include <functional>
#include <mutex>
#include <unordered_map>
#include <variant>

#include <pluginterfaces/vst/ivstparameterchanges.h>
#include <pluginterfaces/vst/vsttypes.h>
#include <public.sdk/source/vst/utility/sampleaccurate.h>
#include <public.sdk/source/vst/vstparameters.h>
#include <vector>

#include "constants.h"
#include "tags.h"

namespace sidebands {

using Steinberg::IPtr;
using Steinberg::Vst::Parameter;
using Steinberg::Vst::ParameterContainer;
using Steinberg::Vst::ParameterInfo;
using Steinberg::Vst::ParamID;
using Steinberg::Vst::ParamValue;
using Steinberg::Vst::RangeParameter;
using Steinberg::Vst::Unit;

class GeneratorPatch : public Steinberg::FObject {
public:
  GeneratorPatch(uint32_t gennum, Steinberg::Vst::UnitID);

  void AppendParameters(ParameterContainer *container);
  void BeginParameterChange(ParamID param_id,
                            Steinberg::Vst::IParamValueQueue *p_queue);
  void EndChanges();
  void AdvanceParameterChanges(uint32_t num_samples);

  void update(FUnknown *changedUnknown, Steinberg::int32 message) override;

  bool on() const;

  // SampleAccurate values get special accessors.
  ParamValue c() const;
  ParamValue a() const;
  ParamValue m() const;
  ParamValue k() const;
  ParamValue r() const;
  ParamValue s() const;
  ParamValue portamento() const;

  struct EnvelopeValues {
    ParamValue A_R; // attack rate
    ParamValue A_L; // attack peak level
    ParamValue D_R; // decay rate
    ParamValue S_L; // sustain level
    ParamValue R_R; // release rate
    ParamValue VS;  // velocity sensitivity
  };
  enum class LFOType { SIN, COS };
  static constexpr LFOType kLFOTypes[]{LFOType::SIN, LFOType::COS};
  static constexpr int kNumLFOTypes = sizeof(kLFOTypes) / sizeof(LFOType);

  struct LFOValues {
    LFOType type;
    ParamValue frequency;
    ParamValue amplitude;
    ParamValue velocity_sensitivty;
  };
  using ModulationParameters = std::variant<EnvelopeValues, LFOValues>;

  enum class ModType { NONE, ENVELOPE, LFO };
  static constexpr ModType kModTypes[]{ModType::NONE, ModType::ENVELOPE,
                                       ModType::LFO};

  static constexpr int kNumModTypes = sizeof(kModTypes) / sizeof(ModType);

  std::optional<ModulationParameters>
  ModulationParams(TargetTag destination) const;
  ModType ModTypeFor(TargetTag destination) const;

  std::function<double()> ParameterGetterFor(TargetTag dest) const;

private:
  IPtr<RangeParameter>
  DeclareParameter(Steinberg::Vst::SampleAccurate::Parameter *param_v,
                   IPtr<RangeParameter> param);
  IPtr<RangeParameter> DeclareParameter(IPtr<RangeParameter> param);
  void DeclareEnvelopeParameters(Steinberg::Vst::UnitID unit_id,
                                 TargetTag target, uint32_t gen_num);

  const uint32_t gennum_;

  mutable std::mutex patch_mutex_;

  IPtr<RangeParameter> on_;
  Steinberg::Vst::SampleAccurate::Parameter c_, a_, m_, k_, r_, s_,
      portamento_;

  IPtr<RangeParameter> mod_type_[NUM_TARGETS];

  struct EnvelopeParameters {
    IPtr<RangeParameter> a_r_;
    IPtr<RangeParameter> a_l_;
    IPtr<RangeParameter> d_r_;
    IPtr<RangeParameter> s_l_;
    IPtr<RangeParameter> r_r_;
    IPtr<RangeParameter> vel_sense_;
  };
  EnvelopeParameters envelope_parameters_[NUM_TARGETS];

  struct LFOParameters {
    IPtr<RangeParameter> frequency_;
    IPtr<RangeParameter> function_;
    IPtr<RangeParameter> amplitude_;
    IPtr<RangeParameter> vel_sense_;
  };
  LFOParameters lfo_parameters_[NUM_TARGETS];

  struct PDesc {
    Steinberg::Vst::SampleAccurate::Parameter *sa_param;
    enum class Type { SAMPLE_ACCURATE, VALUE };
    Type type;
    IPtr<RangeParameter> param;
  };
  std::unordered_map<ParamKey, std::unique_ptr<PDesc>, ParamKey::Hash>
      parameters_;
};


class Patch {
public:
  Patch();

  void AppendParameters(ParameterContainer *container);
  void BeginParameterChange(ParamID param_id,
                            Steinberg::Vst::IParamValueQueue *p_queue);
  void EndParameterChanges();
  void AdvanceParameterChanges(uint32_t num_samples);

  std::unique_ptr<GeneratorPatch> generators_[kNumGenerators];
};

} // namespace sidebands