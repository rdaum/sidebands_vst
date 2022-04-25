#pragma once

#include <pluginterfaces/vst/vsttypes.h>

#include <array>
#include <chrono>
#include <mutex>
#include <vector>

#include "dsp/oscbuffer.h"
#include "globals.h"
#include "processor/synthesis/envgen.h"
#include "processor/synthesis/generator.h"
#include "processor/synthesis/oscillator.h"
#include "processor/synthesis/voice.h"

namespace sidebands {

using Steinberg::Vst::ParamValue;
using Steinberg::Vst::Sample32;
using Steinberg::Vst::Sample64;

// Manages currently playing voices and dispatches note on/noteoff events, and
// fills and mixes audio buffers from playing voices.
class Player {
 public:
  Player(PatchProcessor *patch, SampleRate sample_rate);

  // Fill the audio buffer.
  bool Perform32(Sample32 *in_buffer, Sample32 *out_buffer,
                 size_t frames_per_buffer);
  bool Perform64(Sample64 *in_buffer, Sample64 *out_buffer,
                 size_t frames_per_buffer);
  // Signal note-on to all voices and generators.
  void NoteOn(std::chrono::high_resolution_clock::time_point start_time,
              int32_t note_id, ParamValue velocity, int16_t pitch);

  // Signal note-off.
  void NoteOff(int32_t, int16_t pitch);

  PlayerEvents events;

 private:
  // Allocate a new voice or steal one if necessary.
  Voice *NewVoice(int32_t note_id);
  bool Perform(OscBuffer &buffer);

  const SampleRate sample_rate_;
  PatchProcessor *patch_;  // Current patch.

  // Mutex for locking the voices and their states.
  mutable std::mutex voices_mutex_;

  // The set of voices.
  std::unordered_map<int32_t /* note_id */, Voice> voices_;
};

}  // namespace sidebands