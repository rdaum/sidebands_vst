#pragma once

#include <pluginterfaces/vst/ivstparameterchanges.h>
#include <pluginterfaces/vst/vsttypes.h>

#include <functional>
#include <mutex>
#include <unordered_map>
#include <variant>
#include <vector>

#include "constants.h"
#include "processor/sample_accurate_value.h"
#include "tags.h"

namespace sidebands {



class GeneratorPatch {
 public:
  GeneratorPatch(uint32_t gennum, Steinberg::Vst::UnitID);

  // Invoked from the processor
  void BeginParameterChange(ParamID param_id,
                            Steinberg::Vst::IParamValueQueue *p_queue);
  void EndChanges();
  void AdvanceParameterChanges(uint32_t num_samples);

  // Accessors for various parameters to insure locking.
  bool on() const;
  ParamValue c() const;
  ParamValue a() const;
  ParamValue m() const;
  ParamValue k() const;
  ParamValue r() const;
  ParamValue s() const;
  ParamValue portamento() const;

  struct EnvelopeValues {
    SampleAccurateValue A_R;  // attack rate
    SampleAccurateValue A_L;  // attack peak level
    SampleAccurateValue D_R;  // decay rate
    SampleAccurateValue S_L;  // sustain level
    SampleAccurateValue R_R;  // release rate
    SampleAccurateValue VS;   // velocity sensitivity
  };

  struct LFOValues {
    SampleAccurateValue type;
    SampleAccurateValue frequency;
    SampleAccurateValue amplitude;
    SampleAccurateValue velocity_sensivity;
  };
  using ModulationParameters = std::variant<EnvelopeValues, LFOValues>;


  std::optional<ModulationParameters> ModulationParams(
      TargetTag destination) const;
  ModType ModTypeFor(TargetTag destination) const;

  std::function<double()> ParameterGetterFor(TargetTag dest) const;

 private:
   void DeclareParameter(SampleAccurateValue *value);

  const uint32_t gennum_;

  mutable std::mutex patch_mutex_;

  SampleAccurateValue on_;
  SampleAccurateValue c_, a_, m_, k_, r_, s_, portamento_;
  struct ModTarget {
    SampleAccurateValue mod_type;
    EnvelopeValues envelope_parameters;
    LFOValues lfo_parameters;
  };
  std::vector<ModTarget> mod_targets_;

  std::unordered_map<ParamKey, SampleAccurateValue*, ParamKey::Hash>
      parameters_;
};

class Patch {
 public:
  Patch();

  void BeginParameterChange(ParamID param_id,
                            Steinberg::Vst::IParamValueQueue *p_queue);
  void EndParameterChanges();
  void AdvanceParameterChanges(uint32_t num_samples);

  static bool ValidParam(ParamID param_id);

  std::unique_ptr<GeneratorPatch> generators_[kNumGenerators];
};

}  // namespace sidebands