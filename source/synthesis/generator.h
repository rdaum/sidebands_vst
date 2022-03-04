#pragma once

#include <chrono>
#include <mutex>
#include <pluginterfaces/vst/vsttypes.h>
#include <vector>

#include "synthesis/envgen.h"
#include "synthesis/oscillator.h"

namespace sidebands {

using Steinberg::Vst::SampleRate;

// Each "generator" represents a single FM oscillator + associated envelope
// generators or other modulation sources.
class Generator {
public:
  Generator();
  virtual ~Generator() = default;

  void Perform(SampleRate sample_rate, GeneratorPatch &patch,
               std::valarray<std::complex<double>> &out_buffer,
               Steinberg::Vst::ParamValue base_freq);

  void NoteOn(SampleRate sample_rate, const GeneratorPatch &patch,
              std::chrono::high_resolution_clock::time_point start_time,
              ParamValue velocity, uint8_t note);

  void NoteOff(SampleRate sample_rate, const GeneratorPatch &patch,
               uint8_t note);

  bool Playing() const;
  void Reset();

private:
  void ConfigureModulators(const GeneratorPatch &patch);
  IModulationSource *ModulatorFor(const GeneratorPatch &patch, TargetTag dest);

  std::function<double()> ProducerFor(SampleRate sample_rate,
                                      ParamValue velocity, GeneratorPatch &gp,
                                      TargetTag dest);
  std::function<std::complex<double>()>
  ImaginaryProducerFor(SampleRate sample_rate, ParamValue velocity,
                       GeneratorPatch &gp, TargetTag dest);
  std::unique_ptr<IModulationSource> modulators_[NUM_TARGETS];
  ParamValue velocity_ = 0;
  Oscillator o_;
};

} // namespace sidebands
