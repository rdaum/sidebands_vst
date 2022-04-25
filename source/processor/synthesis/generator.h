#pragma once

#include <pluginterfaces/vst/vsttypes.h>

#include <bitset>
#include <chrono>
#include <mutex>
#include <vector>

#include "globals.h"
#include "processor/events.h"
#include "processor/synthesis/envgen.h"
#include "processor/synthesis/oscillator.h"

namespace sidebands {

using Steinberg::Vst::SampleRate;

// Each "generator" represents a single oscillator + associated envelope
// generators or other modulation sources.
class Generator {
 public:
  Generator();
  virtual ~Generator() = default;

  // Just synthesize, no modulation. For analysis.
  void Synthesize(SampleRate sample_rate, GeneratorPatch &patch,
                  OscBuffer &out_buffer, Steinberg::Vst::ParamValue base_freq);

  // Synthesize and apply modulation and envelope.
  void Perform(SampleRate sample_rate, GeneratorPatch &patch,
               OscBuffer &out_buffer, Steinberg::Vst::ParamValue base_freq);

  void NoteOn(SampleRate sample_rate, const GeneratorPatch &patch,
              std::chrono::high_resolution_clock::time_point start_time,
              ParamValue velocity, uint8_t note);

  void NoteRelease(SampleRate sample_rate, const GeneratorPatch &patch,
                   uint8_t note);

  void Reset();

  GeneratorEvents events;

 private:
  void Produce(SampleRate sample_rate, GeneratorPatch &patch, OscParam &buffer,
               TargetTag target);
  void ConfigureModulators(const GeneratorPatch &patch);

  std::unique_ptr<IModulationSource> modulators_[NUM_TARGETS]
                                                [Modulation::NumModulators];
  ParamValue velocity_ = 0;
  std::unique_ptr<IOscillator> o_;
};

}  // namespace sidebands
