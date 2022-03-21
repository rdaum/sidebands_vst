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
      mod->Amplitudes(sample_rate, mod_a, velocity_, mod_opt);
      VmulInplace(buffer, mod_a);
    }
  }
}

void Generator::Perform(SampleRate sample_rate, GeneratorPatch &patch,
                        OscBuffer &out_buffer,
                        Steinberg::Vst::ParamValue base_freq) {
  auto frames_per_buffer = out_buffer.size();
  OscParams params(frames_per_buffer);

  OscParam A(frames_per_buffer);

  params.note_freq = base_freq;
  Produce(sample_rate, patch, A, TARGET_A);
  Produce(sample_rate, patch, params.K, TARGET_K);
  Produce(sample_rate, patch, params.C, TARGET_C);
  Produce(sample_rate, patch, params.R, TARGET_R);
  Produce(sample_rate, patch, params.S, TARGET_S);
  Produce(sample_rate, patch, params.M, TARGET_M);

  o_->Perform(sample_rate, out_buffer, params);

  // Apply envelope.
  VmulInplace(out_buffer, A);
}

void Generator::NoteOn(
    SampleRate sample_rate, const GeneratorPatch &patch,
    std::chrono::high_resolution_clock::time_point start_time,
    ParamValue velocity, uint8_t note) {
  ConfigureModulators(patch);
  if (!o_ || o_->osc_type() != patch.osc_type()) {
    o_ = MakeOscillator(patch.osc_type());
  }

  velocity_ = velocity;
  for (auto dest : kModulationTargets) {
    auto *modulator = ModulatorFor(patch, dest);
    if (modulator) {
      modulator->On(sample_rate, patch.ModulationParams(dest));
    }
  }
  events.GeneratorOn(this);
}

void Generator::NoteRelease(SampleRate sample_rate, const GeneratorPatch &patch,
                            uint8_t note) {
  for (auto dest : kModulationTargets) {
    auto *modulator = ModulatorFor(patch, dest);
    if (modulator) {
      modulator->Release(sample_rate, patch.ModulationParams(dest));
    }
  }
  events.GeneratorRelease(this);
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
      case ModType::ADSR_ENVELOPE: {
        auto envgen = std::make_unique<EnvelopeGenerator>();
        // When the amplitude envelope is done, this generator is done.
        envgen->events.Done.connect([target, this] {
          if (target == TARGET_A) {
            events.GeneratorOff(this);
          }
        });
        envgen->events.StageChange.connect([target, this, &patch](off_t stage) {
          events.EnvelopeStageChange(patch.gennum(), target, stage);
        });
        modulators_[target][off_t(mod_type)] = std::move(envgen);
      } break;
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