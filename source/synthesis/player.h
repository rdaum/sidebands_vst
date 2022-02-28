#pragma once

#include <array>
#include <chrono>
#include <mutex>
#include <vector>

#include <pluginterfaces/vst/vsttypes.h>

#include "envgen.h"
#include "generator.h"
#include "oscillator.h"
#include "voice.h"

namespace sidebands {

using Steinberg::Vst::ParamValue;
using Steinberg::Vst::Sample32;

// Manages currently playing voices and dispatches note on/noteoff events, and
// fills and mixes audio buffers from playing voices.
class Player {
public:
  static constexpr int kNumVoices = 8; // Must be power of 2.

  Player(Patch *patch, SampleRate sample_rate);

  // Fill the audio buffer.
  bool Perform(Sample32 *in_buffer, Sample32 *out_buffer,
               size_t frames_per_buffer);

  // Signal note-on to all voices and generators.
  void NoteOn(std::chrono::high_resolution_clock::time_point start_time,
              int32_t note_id, ParamValue velocity, uint8_t note);

  // Signal note-off.
  void NoteOff(int32_t, uint8_t note);

private:
  // Allocate a new voice or steal one if necessary.
  Voice *NewVoice(int32_t note_id);

  const SampleRate sample_rate_;
  Patch *patch_; // Current patch.

  // Mutex for locking the voices and their states.
  mutable std::mutex voices_mutex_;

  // The set of voices.
  std::unordered_map<int32_t /* note_id */, Voice> voices_;
};

} // namespace sidebands