#pragma once

#include <pluginterfaces/vst/ivstparameterchanges.h>
#include <pluginterfaces/vst/vsttypes.h>

#include <bitset>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <variant>
#include <vector>

#include "constants.h"
#include "processor/util/parameter.h"
#include "processor/util/sample_accurate_value.h"
#include "tags.h"

namespace Steinberg {
class IBStreamer;
}  // namespace Steinberg

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

  // Accessors for various parameters to ensure locking.
  bool on() const;
  ParamValue c() const;
  ParamValue a() const;
  ParamValue m() const;
  ParamValue k() const;
  ParamValue r() const;
  ParamValue s() const;
  ParamValue portamento() const;
  enum class OscType { MOD_FM, ANALOG };
  OscType osc_type() const;

  struct EnvelopeValues {
    SampleAccurateValue HT, AR, AL, DR1, DL1, DR2, SL, RR1, RL1, RR2;
    SampleAccurateValue VS;  // velocity sensitivity
  };
  struct LFOValues {
    Parameter type;
    SampleAccurateValue frequency;
    SampleAccurateValue amplitude;
    SampleAccurateValue velocity_sensivity;
  };
  struct ModParams {
    TargetTag target;
    BitsetParameter modulations;  // bitset
    EnvelopeValues envelope_parameters;
    LFOValues lfo_parameters;
  };

  GeneratorPatch::ModParams *ModulationParams(TargetTag destination) const;
  std::bitset<Modulation::NumModulators> ModTypesFor(
      TargetTag destination) const;
  std::function<double()> ParameterGetterFor(TargetTag dest) const;
  uint32_t gennum() const { return gennum_; }

 private:
  void DeclareParameter(ProcessorParameterValue *value);

  const uint32_t gennum_;

  mutable std::mutex patch_mutex_;

  Parameter on_;
  SampleAccurateValue c_, a_, m_, k_, r_, s_, portamento_;
  Parameter osc_type_;

  std::unique_ptr<ModParams> mod_targets_[NUM_TARGETS];

  std::unordered_map<ParamKey, ProcessorParameterValue *, ParamKey::Hash>
      parameters_;
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

}  // namespace sidebands