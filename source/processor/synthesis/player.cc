#include "player.h"

#include <glog/logging.h>

#include <cmath>
#include <execution>
#include <mutex>

#include "constants.h"
#include "processor/synthesis/oscillator.h"

namespace sidebands {

Player::Player(PatchProcessor *patch, SampleRate sample_rate)
    : patch_(patch), sample_rate_(sample_rate) {}

bool Player::Perform(OscBuffer &mixdown_buffer) {
  auto &g_patches = patch_->generators_;
  std::vector<MixBuffers> mix_buffers(voices_.size());

  auto frames_per_buffer = mixdown_buffer.size();
  {
    std::lock_guard<std::mutex> player_lock(voices_mutex_);

    // Fill buffers for each voice, in parallel, hopefully.
    auto voice_player = [frames_per_buffer,
                         this](std::pair<const int32_t, Voice> &voice) {
      return voice.second.Perform(sample_rate_, frames_per_buffer, patch_);
    };

    std::transform(std::execution::par_unseq, voices_.begin(), voices_.end(),
                   mix_buffers.begin(), voice_player);
  }

  // Mix down.
  if (!mix_buffers.empty()) {
    for (auto &voice_mix_buffers : mix_buffers) {
      for (auto &voice_mix_buffer : voice_mix_buffers)
        VaddInplace(mixdown_buffer, *voice_mix_buffer);
    }
    return true;
  }

  return false;
}

bool Player::Perform32(Sample32 *in_buffer, Sample32 *out_buffer,
                       size_t frames_per_buffer) {
  memset(out_buffer, 0, frames_per_buffer * sizeof(Sample32));
  MixBuffer mixdown_buffer(0.0f, frames_per_buffer);
  if (Perform(mixdown_buffer)) {
    ToFloat(mixdown_buffer, out_buffer);
  }
  return true;
}

bool Player::Perform64(Sample64 *in_buffer, Sample64 *out_buffer,
                       size_t frames_per_buffer) {
  memset(out_buffer, 0, frames_per_buffer * sizeof(Sample32));
  MixBuffer mixdown_buffer(0.0f, frames_per_buffer);
  if (Perform(mixdown_buffer)) {
    std::memcpy(out_buffer, &mixdown_buffer[0], frames_per_buffer);
  }
  return true;
}

void Player::NoteOn(std::chrono::high_resolution_clock::time_point start_time,
                    int32_t note_id, ParamValue velocity, int16_t pitch) {
  std::lock_guard<std::mutex> player_lock(voices_mutex_);

  // If we were sent a note ID of -1, it means the host is not capable of
  // delivering note ids (and the note-off event will be -1 to correspond.)
  if (note_id == -1) {
    note_id = pitch;
  }
  Voice *v = NewVoice(note_id);
  assert(v != nullptr);
  v->events.EnvelopeStageChange.connect(
      [this, note_id](Voice *v, int gennum, TargetTag target, off_t stage) {
        events.EnvelopeStageChange(note_id, gennum, target, stage);
      });

  // TODO legato, portamento, etc.
  v->NoteOn(sample_rate_, patch_, start_time, velocity, pitch);
}

void Player::NoteOff(int32_t note_id, int16_t pitch) {
  std::lock_guard<std::mutex> player_lock(voices_mutex_);

  // Handle "-1" note IDs on note off on hosts that do that.
  if (note_id == -1) {
    note_id = pitch;
  }

  // Find the voice playing this note id and send it a note-off event.
  auto voice_it = voices_.find(note_id);
  if (voice_it == voices_.end()) {
    LOG(ERROR) << "Unable to find voice for: " << std::hex << note_id;
    return;
  }
  voice_it->second.NoteRelease(sample_rate_, patch_, pitch);
}

Voice *Player::NewVoice(int32_t note_id) {
  if (voices_.size() < kNumVoices) {
    return &voices_[note_id];
  }

  // No free voice? Find the one with the lowest timestamp, shut it off, and
  // allocate a new one.
  std::chrono::high_resolution_clock::time_point least_ts =
      std::chrono::high_resolution_clock::now();

  auto stolen_voice_it = voices_.end();
  for (auto voice_it = voices_.begin(); voice_it != voices_.end(); voice_it++) {
    if (voice_it->second.on_time() < least_ts) {
      stolen_voice_it = voice_it;
      least_ts = voice_it->second.on_time();
    }
  }
  if (stolen_voice_it == voices_.end()) {
    return nullptr;
  }

  stolen_voice_it->second.Reset();
  voices_.erase(stolen_voice_it);

  return &voices_[note_id];
}

}  // namespace sidebands