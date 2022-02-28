#include "synthesis/voice.h"

#include <execution>

#include "synthesis/generator.h"

namespace sidebands {

namespace {

constexpr ParamValue kNoteConversionMultiplier = 440.0f / 32.0f;

ParamValue NoteToFreq(ParamValue note) {
  return kNoteConversionMultiplier * std::pow(2.0f, ((note - 9.0f) / 12.0f));
}

} // namespace

Voice::Voice() : base_freq_(0), note_(0), velocity_(0) {
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

void Voice::NoteOn(SampleRate sample_rate, Patch *patch,
                   std::chrono::high_resolution_clock::time_point start_time,
                   ParamValue velocity, uint8_t note) {
  ParamValue base_freq = NoteToFreq(note);

  note_ = note;
  on_time_ = start_time;
  base_freq_ = base_freq;
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

void Voice::NoteOff(SampleRate sample_rate, Patch *patch, uint8_t note) {
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
                          Patch *patch) {
  if (!Playing())
    return {};
  auto g_patches = patch->generators_;

  // Copy references to the generators that we need to use, and create a mix
  // buffer for each.
  std::vector<std::pair<GeneratorPatch *, Generator *>> generators;
  {
    std::lock_guard<std::mutex> generators_lock(generators_mutex_);
    for (int g_num = 0; g_num < kNumGenerators; g_num++) {
      auto &g = generators_[g_num];
      if (!g->Playing() || !g_patches[g_num]->on())
        continue;
      generators.emplace_back(std::make_pair(g_patches[g_num].get(), g.get()));
    }
  }

  // Perform into the mix buffers and return them all.
  MixBuffers mix_buffers(generators.size());
  std::transform(std::execution::par_unseq, generators.begin(),
                 generators.end(), mix_buffers.begin(),
                 [frames_per_buffer, sample_rate,
                  this](const std::pair<GeneratorPatch *, Generator *> &gpair) {
                   MixBuffer mix_buffer(frames_per_buffer);
                   gpair.second->Perform(sample_rate, *gpair.first,
                                         mix_buffer.data(), base_freq_,
                                         frames_per_buffer);
                   return mix_buffer;
                 });
  return mix_buffers;
}

void Voice::Reset() {
  std::lock_guard<std::mutex> generators_lock(generators_mutex_);
  for (auto &g : generators_) {
    g->Reset();
  }
}

} // namespace sidebands