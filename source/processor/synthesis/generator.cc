#include "processor/synthesis/generator.h"

#include <cmath>
#include <mutex>

#include "constants.h"
#include "processor/synthesis/dsp.h"
#include "processor/synthesis/lfo.h"
#include "processor/synthesis/oscillator.h"

namespace sidebands {

Generator::Generator() = default;

void Generator::Produce(SampleRate sample_rate, GeneratorPatch &patch,
                        OscParam &buffer, TargetTag target) {
  auto value = patch.ParameterGetterFor(target)();
  std::fill(std::begin(buffer), std::end(buffer), value);
  auto mod_opt = patch.ModulationParams(target);
  if (mod_opt) {
    OscBuffer mod_a(buffer.size());
    auto mod = ModulatorFor(patch, target);
    if (mod) {
      std::generate(std::begin(mod_a), std::end(mod_a),
                    [sample_rate, &mod, &mod_opt, &patch, target, this]() {
                      return mod->NextSample(sample_rate, velocity_, mod_opt);
                    });
      VmulInplace(buffer, mod_a);
    }
  }
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

  Produce(sample_rate, patch, A, TARGET_A);
  Produce(sample_rate, patch, K, TARGET_K);
  Produce(sample_rate, patch, C, TARGET_C);
  Produce(sample_rate, patch, R, TARGET_R);
  Produce(sample_rate, patch, S, TARGET_S);
  Produce(sample_rate, patch, M, TARGET_M);

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
      modulator->On(sample_rate, patch.ModulationParams(dest));
    }
  }
}

void Generator::NoteOff(SampleRate sample_rate, const GeneratorPatch &patch,
                        uint8_t note) {
  for (auto dest : kModulationTargets) {
    auto *modulator = ModulatorFor(patch, dest);
    if (modulator) {
      modulator->Release(sample_rate, patch.ModulationParams(dest));
    }
  }
}

bool Generator::Playing() const {
  for (const auto &mod_type : kModTypes) {
    auto &mod = modulators_[TARGET_A][off_t(mod_type)];
    if (mod && mod->Playing())
      return true;
  }
  return false;
}

void Generator::Reset() {
  for (const auto &target : kModulationTargets) {
    for (const auto &mod_type : kModTypes) {
      auto &mod = modulators_[target][off_t(mod_type)];
      if (mod)
        mod->Reset();
    }
  }
}

void Generator::ConfigureModulators(const GeneratorPatch &patch) {
  for (const auto &target : kModulationTargets) {
    for (const auto &mod_type : kModTypes) {
      switch (mod_type) {
      case ModType::NONE:
        modulators_[target][off_t(mod_type)].reset();
        break;
      case ModType::ADSR_ENVELOPE:
        modulators_[target][off_t(mod_type)] =
            std::make_unique<ADSREnvelopeGenerator>();
        break;
      case ModType::LFO:
        modulators_[target][off_t(mod_type)] = std::make_unique<LFO>();
        break;
      }
    }
  }
}

IModulationSource *Generator::ModulatorFor(const GeneratorPatch &patch,
                                           TargetTag dest) {
  return modulators_[dest][off_t(patch.ModTypeFor(dest))].get();
}

} // namespace sidebands