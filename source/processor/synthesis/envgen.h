#pragma once

#include <pluginterfaces/vst/vsttypes.h>

#include <cmath>

#include "dsp/oscbuffer.h"
#include "processor/events.h"
#include "processor/patch_processor.h"
#include "processor/synthesis/modulation_source.h"

namespace sidebands {

using Steinberg::Vst::ParamValue;
using Steinberg::Vst::SampleRate;

namespace ui {
class GraphicalEnvelopeEditorView;
}  // namespace ui

class EnvelopeGenerator : public IModulationSource {
 public:
  explicit EnvelopeGenerator()
      : minimum_level_(0.0001),
        current_stage_(0),
        current_level_(minimum_level_),
        current_sample_index_(0) {}

  // IModulationSource overrides
  void On(SampleRate sample_rate,
          const GeneratorPatch::ModParams *parameters) override;
  void Release(SampleRate sample_rate,
               const GeneratorPatch::ModParams *parameters) override;
  void Reset() override;
  void Amplitudes(SampleRate sample_rate, OscBuffer &buffer,
                  ParamValue velocity,
                  const GeneratorPatch::ModParams *parameters) override;
  bool Playing() const override;
  Modulation::Type mod_type() const override;

  EnvelopeEvents events;

 private:
  ParamValue NextSample(SampleRate sample_rate,
                        const GeneratorPatch::EnvelopeValues &ev);

  struct Stage {
    std::string name;
    double start_level;
    double end_level;
    double coefficient;
    double duration_samples;
  };
  off_t AddStage(double sample_rate, const std::string &name,
                 double start_level, double end_level, double duration);
  void SetStage(off_t stage_number);

  mutable std::mutex stages_mutex_;
  std::vector<Stage> stages_;
  off_t current_stage_ = 0;
  off_t release_stage_ = 0;
  off_t sustain_stage_ = 0;

  const ParamValue minimum_level_;
  double current_level_;
  off_t current_sample_index_;
};

}  // namespace sidebands