#pragma once

#include <pluginterfaces/vst/vsttypes.h>

#include <bitset>
#include <chrono>
#include <complex>
#include <mutex>
#include <valarray>
#include <vector>

#include "constants.h"
#include "globals.h"
#include "processor/events.h"

namespace sidebands {

using Steinberg::Vst::ParamValue;
using Steinberg::Vst::SampleRate;

using MixBuffer = std::valarray<double>;
using MixBuffers = std::vector<std::unique_ptr<MixBuffer>>;

struct PatchProcessor;
class Generator;

class Voice {
 public:
  Voice();

  // Produce a series of buffers, one per playing generator.
  MixBuffers Perform(SampleRate sample_rate, size_t frames_per_buffer,
                     PatchProcessor *patch);

  // Trigger a note-on even for each generator in the voice.
  void NoteOn(SampleRate sample_rate, PatchProcessor *patch,
              std::chrono::high_resolution_clock::time_point start_time,
              ParamValue velocity, int16_t note);

  // Trigger a note-release for each generator in the voice.
  void NoteRelease(SampleRate sample_rate, PatchProcessor *patch, int16_t note);

  // Force all off (i.e. for stealing)
  void Reset();

  // Returns true if the voice is generating sound (envelopes for any of its
  // generators are active).
  bool Playing() const;

  int16_t note() const { return note_; }
  std::chrono::high_resolution_clock::time_point on_time() const {
    return on_time_;
  }

  VoiceEvents events;

 private:
  mutable std::mutex generators_mutex_;
  std::unique_ptr<Generator> generators_[kNumGenerators];
  std::bitset<kNumGenerators> active_generators_;
  std::chrono::high_resolution_clock::time_point on_time_;
  int16_t note_;
  ParamValue velocity_;
  ParamValue note_frequency_;
};

}  // namespace sidebands