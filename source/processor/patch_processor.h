#pragma once

#include <pluginterfaces/vst/ivstparameterchanges.h>
#include <pluginterfaces/vst/vsttypes.h>

#include <functional>
#include <mutex>
#include <unordered_map>
#include <variant>
#include <vector>

#include "constants.h"
#include "processor/util/sample_accurate_value.h"
#include "tags.h"

namespace Steinberg {
class IBStreamer;
} // namespace Steinberg

namespace sidebands {

// Parameter value storage for patch parameters for each generator.
class GeneratorPatch {
public:
  GeneratorPatch(uint32_t gennum, Steinberg::Vst::UnitID);

  // Invoked from the processor
  void BeginParameterChange(ParamID param_id,
                            Steinberg::Vst::IParamValueQueue *p_queue);
  void EndChanges();
  void AdvanceParameterChanges(uint32_t num_samples);

  Steinberg::tresult LoadPatch(Steinberg::IBStreamer &stream);
  Steinberg::tresult SavePatch(Steinberg::IBStreamer &stream);

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
    SampleAccurateValue A_R; // attack rate
    SampleAccurateValue A_L; // attack peak level
    SampleAccurateValue D_R; // decay rate
    SampleAccurateValue S_L; // sustain level
    SampleAccurateValue R_R; // release rate
    SampleAccurateValue VS;  // velocity sensitivity
  };
  struct LFOValues {
    ParamValue type;
    SampleAccurateValue frequency;
    SampleAccurateValue amplitude;
    SampleAccurateValue velocity_sensivity;
  };
  using ModulationParameters = std::variant<EnvelopeValues, LFOValues>;

  std::optional<ModulationParameters>
  ModulationParams(TargetTag destination) const;
  ModType ModTypeFor(TargetTag destination) const;

  std::function<double()> ParameterGetterFor(TargetTag dest) const;

private:
  void DeclareParameter(SampleAccurateValue *value);
  void DeclareParameter(ParamID param_id, Steinberg::Vst::ParamValue *value,
                        ParamValue min, ParamValue max);

  const uint32_t gennum_;

  mutable std::mutex patch_mutex_;

  ParamValue on_;
  SampleAccurateValue c_, a_, m_, k_, r_, s_, portamento_;
  struct ModTarget {
    TargetTag target;
    ParamValue mod_type;
    EnvelopeValues envelope_parameters;
    LFOValues lfo_parameters;
  };
  std::unique_ptr<ModTarget> mod_targets_[NUM_TARGETS];

  struct Param {
    union {
      ParamValue *v;
      SampleAccurateValue *sv;
    } v;
    bool sa;
    ParamValue min;
    ParamValue max;
  };
  std::unordered_map<ParamKey, Param, ParamKey::Hash> parameters_;
};

// Manages the processing of parameter changes from the processor loop.
class PatchProcessor {
public:
  PatchProcessor();

  void BeginParameterChange(ParamID param_id,
                            Steinberg::Vst::IParamValueQueue *p_queue);
  void EndParameterChanges();
  void AdvanceParameterChanges(uint32_t num_samples);

  Steinberg::tresult LoadPatch(Steinberg::IBStream *stream);
  Steinberg::tresult SavePatch(Steinberg::IBStream *stream);

  static bool ValidParam(ParamID param_id);

  std::unique_ptr<GeneratorPatch> generators_[kNumGenerators];
};

} // namespace sidebands