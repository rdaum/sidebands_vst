#include "processor/synthesis/voice.h"

#include <execution>

#include "processor/synthesis/generator.h"

namespace sidebands {

namespace {

constexpr ParamValue kNoteConversionMultiplier = 440.0f / 32.0f;

ParamValue NoteToFreq(ParamValue note) {
  return kNoteConversionMultiplier * std::pow(2.0f, ((note - 9.0f) / 12.0f));
}

} // namespace

Voice::Voice() : note_frequency_(0), note_(0), velocity_(0) {
  for (int x = 0; x < kNumGenerators; x++) {
    generators_[x] = std::make_unique<Generator>();
  }
}

bool Voice::Playing() const {
  std::lock_guard<std::mutex> generators_lock(generators_mutex_);

  for (const auto &g : generators_) {
    if (g->Playing())
      return true;
  }
  return false;
}

void Voice::NoteOn(SampleRate sample_rate, PatchProcessor *patch,
                   std::chrono::high_resolution_clock::time_point start_time,
                   ParamValue velocity, int16_t note) {
  ParamValue base_freq = NoteToFreq(note);

  note_ = note;
  on_time_ = start_time;
  note_frequency_ = base_freq;
  velocity_ = velocity;

  std::lock_guard<std::mutex> generators_lock(generators_mutex_);
  auto &g_patches = patch->generators_;
  for (int g_num = 0; g_num < kNumGenerators; g_num++) {
    auto &g = this->generators_[g_num];
    auto &gp = g_patches[g_num];
    if (gp->on()) {
      g->NoteOn(sample_rate, *gp, start_time, velocity_, note);
    }
  }
}

void Voice::NoteOff(SampleRate sample_rate, PatchProcessor *patch,
                    int16_t note) {
  auto &g_patches = patch->generators_;

  std::lock_guard<std::mutex> generators_lock(generators_mutex_);
  for (int g_num = 0; g_num < kNumGenerators; g_num++) {
    auto &g = generators_[g_num];
    if (!g->Playing())
      continue;
    auto &gp = g_patches[g_num];
    g->NoteOff(sample_rate, *gp, note);
  }
}

MixBuffers Voice::Perform(SampleRate sample_rate, size_t frames_per_buffer,
                          PatchProcessor *patch) {
  if (!Playing())
    return {};
  auto g_patches = patch->generators_;

  // Copy references to the generators that we need to use, and create a mix
  // buffer for each.
  std::vector<std::pair<GeneratorPatch *, Generator *>> generators;
  {
    std::lock_guard<std::mutex> generators_lock(generators_mutex_);
    current_state.active_generators.reset();
    for (int g_num = 0; g_num < kNumGenerators; g_num++) {
      auto &g = generators_[g_num];
      if (!g->Playing() || !g_patches[g_num]->on())
        continue;
      current_state.active_generators[g_num] = true;
      generators.emplace_back(std::make_pair(g_patches[g_num].get(), g.get()));
    }
  }

  // Perform into the mix buffers and return them all.
  MixBuffers mix_buffers(generators.size());
  std::transform(std::execution::par_unseq, generators.begin(),
                 generators.end(), mix_buffers.begin(),
                 [frames_per_buffer, sample_rate,
                  this](const std::pair<GeneratorPatch *, Generator *> &gpair) {
                   auto mix_buffer =
                       std::make_unique<MixBuffer>(frames_per_buffer);
                   gpair.second->Perform(sample_rate, *gpair.first,
                                         *mix_buffer.get(), note_frequency_);
                   return mix_buffer;
                 });

  current_state.level = 0;
  for (const auto &mix_buffer : mix_buffers) {
    current_state.level += (*mix_buffer)[0];
  }
  return mix_buffers;
}

void Voice::Reset() {
  std::lock_guard<std::mutex> generators_lock(generators_mutex_);
  for (auto &g : generators_) {
    g->Reset();
  }
}

void Voice::UpdateState(PlayerState::VoiceState *voice_state) {
  // Poll generators for active state.
  for (int g_num = 0; g_num < kNumGenerators; g_num++) {
    if (current_state.active_generators.test(g_num)) {
      std::lock_guard<std::mutex> generators_lock(generators_mutex_);
      const auto &generator = generators_[g_num];
      generator->UpdateState(&current_state.generator_states[g_num]);
    }
  }

  // Copy over.
  *voice_state = current_state;
}

} // namespace sidebands