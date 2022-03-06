#pragma once

#include <pluginterfaces/vst/vsttypes.h>

#include <cmath>

#include "synthesis/modulation_source.h"
#include "synthesis/patch.h"

namespace sidebands {

using Steinberg::Vst::ParamValue;
using Steinberg::Vst::SampleRate;

class EnvelopeGenerator : public IModulationSource {
 public:
  explicit EnvelopeGenerator()
      : minimum_level_(0.0001),
        stage_(ENVELOPE_STAGE_OFF),
        current_level_(minimum_level_),
        coefficient_(1.0),
        current_sample_index_(0),
        next_stage_sample_index_(0){};

  enum EnvelopeStage {
    ENVELOPE_STAGE_OFF = 0,
    ENVELOPE_STAGE_ATTACK,
    ENVELOPE_STAGE_DECAY,
    ENVELOPE_STAGE_SUSTAIN,
    ENVELOPE_STAGE_RELEASE,
    kNumEnvelopeStages
  };

  inline EnvelopeStage stage() const { return stage_; };

  // IModulationSource overrides
  void On(SampleRate sample_rate,
          const GeneratorPatch::ModulationParameters &parameters) override;
  void Release(SampleRate sample_rate,
               const GeneratorPatch::ModulationParameters &parameters) override;
  void Reset() override;
  ParamValue NextSample(
      SampleRate sample_rate, ParamValue velocity,
      const GeneratorPatch::ModulationParameters &parameters) override;
  bool Playing() const override { return stage_ != ENVELOPE_STAGE_OFF; }
  GeneratorPatch::ModType mod_type() const override;

 private:
  void EnterStage(SampleRate sample_rate, EnvelopeStage new_stage,
                  const GeneratorPatch::EnvelopeValues &envelope);

  const ParamValue minimum_level_;
  EnvelopeStage stage_;
  double current_level_;
  double coefficient_;
  size_t current_sample_index_;
  size_t next_stage_sample_index_;
};

}  // namespace sidebands