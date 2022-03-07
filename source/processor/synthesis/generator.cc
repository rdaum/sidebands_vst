#include "processor/synthesis/generator.h"

#include <cmath>
#include <mutex>

#include "constants.h"
#include "processor/synthesis/dsp.h"
#include "processor/synthesis/lfo.h"
#include "processor/synthesis/oscillator.h"

namespace sidebands {

Generator::Generator() {}

double Produce(SampleRate sample_rate, ParamValue velocity,
               GeneratorPatch &patch, TargetTag destination,
               std::function<double()> value_getter,
               IModulationSource *mod_source) {
  auto value = value_getter();
  auto mod = patch.ModulationParams(destination);
  if (mod.has_value() && mod_source) {
    value *= mod_source->NextSample(sample_rate, velocity, mod.value());
  }
  return value;
}

std::function<std::complex<double>()> Generator::ImaginaryProducerFor(
    SampleRate sample_rate, ParamValue velocity, GeneratorPatch &gp,
    TargetTag dest) {
  return [sample_rate, &gp, dest, this, velocity]() {
    return std::complex<double>(
        0, Produce(sample_rate, velocity, gp, dest, gp.ParameterGetterFor(dest),
                   ModulatorFor(gp, dest)));
  };
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
                        OscBuffer &out_buffer,
                        Steinberg::Vst::ParamValue base_freq) {
  auto frames_per_buffer = out_buffer.size();
  OscParam A(frames_per_buffer);
  OscParam K(frames_per_buffer);
  OscParam C(frames_per_buffer);
  OscParam R(frames_per_buffer);
  OscParam S(frames_per_buffer);
  OscParam M(frames_per_buffer);
  OscParam freq(base_freq, frames_per_buffer);

  std::generate(std::begin(A), std::end(A),
                ProducerFor(sample_rate, velocity_, patch, TARGET_A));
  std::generate(std::begin(K), std::end(K),
                ProducerFor(sample_rate, velocity_, patch, TARGET_K));
  std::generate(std::begin(C), std::end(C),
                ProducerFor(sample_rate, velocity_, patch, TARGET_C));
  std::generate(std::begin(R), std::end(R),
                ProducerFor(sample_rate, velocity_, patch, TARGET_R));
  std::generate(std::begin(S), std::end(S),
                ProducerFor(sample_rate, velocity_, patch, TARGET_S));
  std::generate(std::begin(M), std::end(M),
                ProducerFor(sample_rate, velocity_, patch, TARGET_M));
  o_.Perform(sample_rate, out_buffer, freq, C, M, R, S, K);

  // Apply envelope.
  VmulInplace(out_buffer, A);
}

void Generator::NoteOn(
    SampleRate sample_rate, const GeneratorPatch &patch,
    std::chrono::high_resolution_clock::time_point start_time,
    ParamValue velocity, uint8_t note) {
  ConfigureModulators(patch);

  velocity_ = velocity;
  for (auto dest : kModulationTargets) {
    auto *modulator = ModulatorFor(patch, dest);
    if (modulator) {
      modulator->On(sample_rate, patch.ModulationParams(dest).value());
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
  if (mod)
    return mod->Playing();
  else
    return false;
//  return mod && mod->Playing();
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
      case ModType::NONE:
        modulators_[target].reset();
        break;
      case ModType::ENVELOPE:
        modulators_[target] = std::make_unique<EnvelopeGenerator>();
        break;
      case ModType::LFO:
        modulators_[target] = std::make_unique<LFO>();
        break;
    }
  }
}

IModulationSource *Generator::ModulatorFor(const GeneratorPatch &patch,
                                           TargetTag dest) {
  auto *mod = modulators_[dest].get();
  if (mod && mod->mod_type() != patch.ModTypeFor(dest)) {
    ConfigureModulators(patch);
    mod = modulators_[dest].get();
  }
  return mod;
}

}  // namespace sidebands