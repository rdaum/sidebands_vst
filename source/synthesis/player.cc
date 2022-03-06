#include "player.h"

#include <glog/logging.h>

#include <cmath>
#include <execution>
#include <mutex>

#include "constants.h"
#include "synthesis/dsp.h"
#include "synthesis/oscillator.h"

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
    MixBuffer mixdown_buffer(0.0f, frames_per_buffer);
    for (auto &voice_mix_buffers : mix_buffers) {
      for (auto &voice_mix_buffer : voice_mix_buffers)
        VaddInplace(mixdown_buffer, *voice_mix_buffer);
    }

    ToFloat(mixdown_buffer, out_buffer);
  }
  return true;
}

void Player::NoteOn(std::chrono::high_resolution_clock::time_point start_time,
                    int32_t note_id, ParamValue velocity, int16_t pitch) {
  std::lock_guard<std::mutex> player_lock(voices_mutex_);

  // If we were sent a note ID of -1, it means the host is not capable of
  // delivering note ids (and the note-off event will be -1 or 0 to correspond.)

  // In this case we need to produce some form of synthetic note id, so we'll
  // just use the note # but that means we can't play the same note in sequence
  // while the previous one is releasing.
  // it's just awful, hosts that do this are vile.
  if (note_id == -1 || note_id == 0) {
    if (pitch == 0) {
      LOG(INFO) << "Invalid note id: " << note_id << " note: " << pitch;
      return;
    }
    LOG(INFO) << "On note id: " << std::hex << note_id << " for note: " << (int)pitch;
    note_id = pitch;
  }
  Voice *v = NewVoice(note_id);
  assert(v != nullptr);
  // TODO legato, portamento, etc.
  v->NoteOn(sample_rate_, patch_, start_time, velocity, pitch);
}

void Player::NoteOff(int32_t note_id, int16_t pitch) {
  std::lock_guard<std::mutex> player_lock(voices_mutex_);

  //
  if (note_id == -1 || note_id == 0) {
    for (auto &v : voices_) {
      if (v.second.note() == pitch) {
        LOG(INFO) << "Off note id: " << note_id << " substituted with " << pitch;
        note_id = pitch;
        break;
      }
    }

    if (note_id == -1 || note_id == 0) {
      LOG(INFO) << "Could not find note for fake note id : " << note_id << " " << pitch;
    }
  }

  // Find the voice playing this note id and send it a note-off event.
  auto voice_it = voices_.find(note_id);
  if (voice_it == voices_.end()) {
    LOG(ERROR) << "Unable to find voice for: " << std::hex << note_id;
    return;
  }
  voice_it->second.NoteOff(sample_rate_, patch_, pitch);
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