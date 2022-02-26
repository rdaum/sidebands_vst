#include "player.h"

#include <glog/logging.h>

#include <cmath>
#include <execution>
#include <mutex>

#include "constants.h"
#include "oscillator.h"

namespace sidebands {

Player::Player(Patch *patch, SampleRate sample_rate)
    : patch_(patch), sample_rate_(sample_rate) {}

bool Player::Perform(Sample32 *in_buffer, Sample32 *out_buffer,
                     size_t frames_per_buffer) {
  memset(out_buffer, 0, frames_per_buffer * sizeof(Sample32));

  auto &g_patches = patch_->generators_;
  std::vector<MixBuffers> mix_buffers(voices_.size());

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
    MixBuffer mixdown_buffer(frames_per_buffer, 0.0f);
    for (auto &voice_mix_buffers : mix_buffers) {
      for (int i = 0; i < frames_per_buffer; i++) {
        for (auto &mix_buffer : voice_mix_buffers)
          mixdown_buffer[i] += mix_buffer[i];
      }
    }

    for (int i = 0; i < frames_per_buffer; i++) {
      out_buffer[i] = mixdown_buffer[i].real();
    }
  }
  return true;
}

void Player::NoteOn(std::chrono::high_resolution_clock::time_point start_time,
                    int32_t note_id, uint8_t velocity, uint8_t note) {
  // A note with no velocity is not a note at all.
  if (!velocity)
    return;

  std::lock_guard<std::mutex> player_lock(voices_mutex_);

  Voice *v = NewVoice(note_id);
  assert(v != nullptr);
  // TODO legato, portamento, etc.
  v->NoteOn(sample_rate_, patch_, start_time, velocity, note);
}

void Player::NoteOff(int32_t note_id, uint8_t note) {
  std::lock_guard<std::mutex> player_lock(voices_mutex_);

  // Find the voice playing this note id and send it a note-off event.
  auto voice_it = voices_.find(note_id);
  if (voice_it == voices_.end()) {
    LOG(ERROR) << "Unable to find voice for: " << std::hex << note_id;
    return;
  }
  voice_it->second.NoteOff(sample_rate_, patch_, note);
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

} // namespace sidebands