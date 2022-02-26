#include "generator.h"
#include "constants.h"
#include "oscillator.h"

#include <cmath>
#include <execution>
#include <mutex>

namespace sidebands {

Generator::Generator() {}

void Generator::ConfigureModulators(const GeneratorPatch &patch) {
  for (const auto &target : kModulationTargets) {
    auto mod_type = patch.ModTypeFor(target);
    switch (mod_type) {
    case GeneratorPatch::ModType::NONE:
      modulators_[target].reset();
      break;
    case GeneratorPatch::ModType::ENVELOPE:
      modulators_[target] = std::make_unique<EnvelopeGenerator>();
      break;
    case GeneratorPatch::ModType::LFO:
      // TODO impl.
      modulators_[target].reset();
      break;
    }
  }
}

double Produce(SampleRate sample_rate, GeneratorPatch &patch,
               TargetTag destination, std::function<double()> value_getter,
               IModulationSource *mod_source) {
  auto value = value_getter();
  auto mod = patch.ModulationParams(destination);
  if (mod.has_value() && mod_source) {
    value *= mod_source->NextSample(sample_rate, mod.value());
  }
  return value;
}

std::function<double()> Generator::ProducerFor(SampleRate sample_rate,
                                               GeneratorPatch &gp,
                                               TargetTag dest) const {
  return [sample_rate, &gp, dest, this]() {
    return Produce(sample_rate, gp, dest, gp.ParameterGetterFor(dest),
                   ModulatorFor(dest));
  };
}

void Generator::Perform(SampleRate sample_rate, GeneratorPatch &patch,
                        std::complex<Steinberg::Vst::ParamValue> *out_buffer,
                        Steinberg::Vst::ParamValue base_freq,
                        size_t frames_per_buffer) {
  std::vector<Steinberg::Vst::ParamValue> level_a(frames_per_buffer);
  std::vector<Steinberg::Vst::ParamValue> level_k(frames_per_buffer);
  std::vector<Steinberg::Vst::ParamValue> level_c(frames_per_buffer);
  std::vector<Steinberg::Vst::ParamValue> level_r(frames_per_buffer);
  std::vector<Steinberg::Vst::ParamValue> level_s(frames_per_buffer);
  std::vector<Steinberg::Vst::ParamValue> level_m(frames_per_buffer);

  std::generate(level_a.begin(), level_a.begin() + frames_per_buffer,
                ProducerFor(sample_rate, patch, TARGET_A));
  std::generate(level_k.begin(), level_k.begin() + frames_per_buffer,
                ProducerFor(sample_rate, patch, TARGET_K));
  std::generate(level_c.begin(), level_c.begin() + frames_per_buffer,
                ProducerFor(sample_rate, patch, TARGET_C));
  std::generate(level_r.begin(), level_r.begin() + frames_per_buffer,
                ProducerFor(sample_rate, patch, TARGET_R));
  std::generate(level_s.begin(), level_s.begin() + frames_per_buffer,
                ProducerFor(sample_rate, patch, TARGET_S));
  std::generate(level_m.begin(), level_m.begin() + frames_per_buffer,
                ProducerFor(sample_rate, patch, TARGET_M));

  o_.Perform(frames_per_buffer, sample_rate, out_buffer, base_freq,
             level_a.data(), level_c.data(), level_m.data(), level_r.data(),
             level_s.data(), level_k.data());
}

void Generator::NoteOn(
    SampleRate sample_rate, const GeneratorPatch &patch,
    std::chrono::high_resolution_clock::time_point start_time, uint8_t velocity,
    uint8_t note) {

  ConfigureModulators(patch);

  for (auto dest : kModulationTargets) {
    auto modulator = ModulatorFor(dest);
    if (modulator) {
      modulator->On(sample_rate, patch.ModulationParams(dest).value());
    }
  }
}

void Generator::NoteOff(SampleRate sample_rate, const GeneratorPatch &patch,
                        uint8_t note) {
  for (auto dest : kModulationTargets) {
    auto modulator = ModulatorFor(dest);
    if (modulator) {
      modulator->Release(sample_rate, patch.ModulationParams(dest).value());
    }
  }
}

bool Generator::Playing() const {
  auto &mod = modulators_[TARGET_A];
  return mod && mod->Playing();
}

void Generator::Reset() {
  for (auto dest : kModulationTargets) {
    auto modulator = ModulatorFor(dest);
    if (modulator) {
      modulator->Reset();
    }
  }
}

IModulationSource *Generator::ModulatorFor(TargetTag dest) const {
  return modulators_[dest].get();
}

} // namespace sidebands