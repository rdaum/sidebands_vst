#include "constants.h"
#include "synthesis/generator.h"
#include "synthesis/oscillator.h"
#include "synthesis/lfo.h"

#include <cmath>
#include <execution>
#include <mutex>

namespace sidebands {

Generator::Generator() {}

double Produce(SampleRate sample_rate, ParamValue velocity,
               GeneratorPatch &patch,
               TargetTag destination, std::function<double()> value_getter,
               IModulationSource *mod_source) {
  auto value = value_getter();
  auto mod = patch.ModulationParams(destination);
  if (mod.has_value() && mod_source) {
    value *= mod_source->NextSample(sample_rate, velocity,mod.value());
  }
  return value;
}

std::function<double()> Generator::ProducerFor(SampleRate sample_rate,
                                               ParamValue velocity,
                                               GeneratorPatch &gp,
                                               TargetTag dest) {
  return [sample_rate, &gp, dest, this, velocity]() {
    return Produce(sample_rate, velocity, gp, dest, gp.ParameterGetterFor(dest),
                   ModulatorFor(gp, dest));
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
                ProducerFor(sample_rate, velocity_, patch, TARGET_A));
  std::generate(level_k.begin(), level_k.begin() + frames_per_buffer,
                ProducerFor(sample_rate, velocity_, patch, TARGET_K));
  std::generate(level_c.begin(), level_c.begin() + frames_per_buffer,
                ProducerFor(sample_rate, velocity_, patch, TARGET_C));
  std::generate(level_r.begin(), level_r.begin() + frames_per_buffer,
                ProducerFor(sample_rate, velocity_, patch, TARGET_R));
  std::generate(level_s.begin(), level_s.begin() + frames_per_buffer,
                ProducerFor(sample_rate, velocity_, patch, TARGET_S));
  std::generate(level_m.begin(), level_m.begin() + frames_per_buffer,
                ProducerFor(sample_rate, velocity_, patch, TARGET_M));


  o_.Perform(frames_per_buffer, sample_rate, out_buffer, base_freq,
             level_a.data(), level_c.data(), level_m.data(), level_r.data(),
             level_s.data(), level_k.data());
}

void Generator::NoteOn(
    SampleRate sample_rate, const GeneratorPatch &patch,
    std::chrono::high_resolution_clock::time_point start_time, ParamValue velocity,
    uint8_t note) {

  ConfigureModulators(patch);

  velocity_ = velocity;
  for (auto dest : kModulationTargets) {
    auto *modulator = ModulatorFor(patch, dest);
    if (modulator) {
      modulator->On(sample_rate,  patch.ModulationParams(dest).value());
    }
  }
}

void Generator::NoteOff(SampleRate sample_rate, const GeneratorPatch &patch,
                        uint8_t note) {
  for (auto dest : kModulationTargets) {
    auto *modulator = ModulatorFor(patch, dest);
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
  for (auto &mod : modulators_) {
    if (mod) mod->Reset();
  }
}


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
      modulators_[target] = std::make_unique<LFO>();
      break;
    }
  }
}

IModulationSource *Generator::ModulatorFor(const GeneratorPatch &patch, TargetTag dest) {
  auto *mod = modulators_[dest].get();
  if (mod && mod->mod_type() != patch.ModTypeFor(dest)) {
    ConfigureModulators(patch);
    mod =  modulators_[dest].get();
  }
  return mod;
}

} // namespace sidebands