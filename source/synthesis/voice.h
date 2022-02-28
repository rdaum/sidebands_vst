#pragma once

#include <chrono>
#include <complex>
#include <mutex>
#include <vector>

#include <pluginterfaces/vst/vsttypes.h>

#include "constants.h"

namespace sidebands {

using Steinberg::Vst::ParamValue;
using Steinberg::Vst::SampleRate;

using MixBuffer = std::vector<std::complex<double>>;
using MixBuffers = std::vector<MixBuffer>;

struct Patch;
class Generator;

class Voice {
public:
  Voice();

  // Produce a series of buffers, one per playing generator.
  MixBuffers Perform(SampleRate sample_rate, size_t frames_per_buffer,
                     Patch *patch);

  // Trigger a note-on even for each generator in the voice.
  void NoteOn(SampleRate sample_rate, Patch *patch,
              std::chrono::high_resolution_clock::time_point start_time,
              ParamValue velocity, uint8_t note);

  // Trigger a note-release for each generator in the voice.
  void NoteOff(SampleRate sample_rate, Patch *patch, uint8_t note);

  // Force all off (i.e. for stealing)
  void Reset();

  // Returns true if the voice is generating sound (envelopes for any of its
  // generators are active).
  bool Playing() const;

  uint8_t note() const { return note_; }
  std::chrono::high_resolution_clock::time_point on_time() const {
    return on_time_;
  }

private:
  mutable std::mutex generators_mutex_;
  std::unique_ptr<Generator> generators_[kNumGenerators];
  std::chrono::high_resolution_clock::time_point on_time_;
  uint8_t note_;
  ParamValue velocity_;
  ParamValue base_freq_;
};

} // namespace sidebands