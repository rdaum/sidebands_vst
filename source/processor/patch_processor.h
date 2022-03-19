#pragma once

#include <deque>
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

    struct ADSREnvelopeValues {
    SampleAccurateValue HT, AR, DR1, DR2, RR1, RR2;
    SampleAccurateValue     AL, DL1, SL, RL1;
    SampleAccurateValue VS;  // velocity sensitivity
  };
  struct LFOValues {
    ParamValue type;
    SampleAccurateValue frequency;
    SampleAccurateValue amplitude;
    SampleAccurateValue velocity_sensivity;
  };
  struct ModParams {
    TargetTag target;
    ParamValue mod_type;
    ADSREnvelopeValues adsr_parameters;
    LFOValues lfo_parameters;
  };

  GeneratorPatch::ModParams *ModulationParams(TargetTag destination) const;
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

  std::unique_ptr<ModParams> mod_targets_[NUM_TARGETS];

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
  std::deque<Steinberg::Vst::ParamID> sa_changed_params_;
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