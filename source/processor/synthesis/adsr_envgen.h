#pragma once

#include <pluginterfaces/vst/vsttypes.h>

#include <cmath>

#include "processor/patch_processor.h"
#include "processor/synthesis/modulation_source.h"
#include "dsp.h"

namespace sidebands {

using Steinberg::Vst::ParamValue;
using Steinberg::Vst::SampleRate;

namespace ui {
class GraphicalEnvelopeEditorView;
}  // namespace ui

class ADSREnvelopeGenerator : public IModulationSource {
public:
  explicit ADSREnvelopeGenerator()
      : minimum_level_(0.0001), stage_(ENVELOPE_STAGE_OFF),
        current_level_(minimum_level_), coefficient_(1.0),
        current_sample_index_(0), next_stage_sample_index_(0){};



  // IModulationSource overrides
  void On(SampleRate sample_rate,
          const GeneratorPatch::ModParams *parameters) override;
  void Release(SampleRate sample_rate,
               const GeneratorPatch::ModParams *parameters) override;
  void Reset() override;
  void Amplitudes(SampleRate sample_rate, OscBuffer &buffer,
                          ParamValue velocity,
                          const GeneratorPatch::ModParams *parameters) override;
  bool Playing() const override { return stage_ != ENVELOPE_STAGE_OFF; }
  ModType mod_type() const override;

private:
  enum EnvelopeStage {
    ENVELOPE_STAGE_OFF = 0,
    ENVELOPE_STAGE_ATTACK,
    ENVELOPE_STAGE_DECAY,
    ENVELOPE_STAGE_SUSTAIN,
    ENVELOPE_STAGE_RELEASE,
    kNumEnvelopeStages
  };

  void EnterStage(SampleRate sample_rate, EnvelopeStage new_stage,
                  const GeneratorPatch::ADSREnvelopeValues &envelope);
  ParamValue NextSample(SampleRate sample_rate,
                        const GeneratorPatch::ADSREnvelopeValues &ev) ;
  inline EnvelopeStage stage() const { return stage_; };

  const ParamValue minimum_level_;
  EnvelopeStage stage_;
  double current_level_;
  double coefficient_;
  size_t current_sample_index_;
  size_t next_stage_sample_index_;
};

} // namespace sidebands