#pragma once

#include <pluginterfaces/vst/vsttypes.h>

#include <chrono>
#include <mutex>
#include <vector>

#include "processor/synthesis/envgen.h"
#include "processor/synthesis/oscillator.h"

namespace sidebands {

using Steinberg::Vst::SampleRate;

// Each "generator" represents a single FM oscillator + associated envelope
// generators or other modulation sources.
class Generator {
 public:
  Generator();
  virtual ~Generator() = default;

  void Perform(SampleRate sample_rate, GeneratorPatch &patch,
               OscBuffer &out_buffer, Steinberg::Vst::ParamValue base_freq);

  void NoteOn(SampleRate sample_rate, const GeneratorPatch &patch,
              std::chrono::high_resolution_clock::time_point start_time,
              ParamValue velocity, uint8_t note);

  void NoteOff(SampleRate sample_rate, const GeneratorPatch &patch,
               uint8_t note);

  bool Playing() const;
  void Reset();

 private:
  void Produce(SampleRate sample_rate, GeneratorPatch &patch, OscParam &buffer, TargetTag target);
  void ConfigureModulators(const GeneratorPatch &patch);
  IModulationSource *ModulatorFor(const GeneratorPatch &patch, TargetTag dest);

  std::unique_ptr<IModulationSource> modulators_[NUM_TARGETS][kNumModTypes];
  ParamValue velocity_ = 0;
  Oscillator o_;
};

}  // namespace sidebands
