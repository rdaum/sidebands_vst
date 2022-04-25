#include "processor/synthesis/generator.h"

#include <cmath>
#include <mutex>

#include "constants.h"
#include "dsp/oscbuffer.h"
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
    auto mod_types = patch.ModTypesFor(target);
    for (int i = 0; i < Modulation::NumModulators; i++) {
      Modulation::Type mod_type = Modulation::Type(i);
      if (mod_types.test(mod_type)) {
        auto &modulator = modulators_[target][mod_type];
        if (modulator) {
          modulator->Amplitudes(sample_rate, mod_a, velocity_, mod_opt);
          VmulInplace(buffer, mod_a);
        }
      }
    }
  }
}

void Generator::Synthesize(SampleRate sample_rate, GeneratorPatch &patch,
                           OscBuffer &out_buffer,
                           Steinberg::Vst::ParamValue base_freq) {
  auto frames_per_buffer = out_buffer.size();
  OscParams params(frames_per_buffer);

  params.note_freq = base_freq;
  params.K = patch.ParameterGetterFor(TARGET_K)();
  params.C = patch.ParameterGetterFor(TARGET_C)();
  params.R = patch.ParameterGetterFor(TARGET_R)();
  params.S = patch.ParameterGetterFor(TARGET_S)();
  params.M = patch.ParameterGetterFor(TARGET_M)();

  auto o = MakeOscillator(patch.osc_type());
  o->Perform(sample_rate, out_buffer, params);
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
  for (auto target : kModulationTargets) {
    auto mod_types = patch.ModTypesFor(target);
    for (int i = 0; i < Modulation::NumModulators; i++) {
      Modulation::Type mod_type = Modulation::Type(i);
      if (mod_types.test(mod_type)) {
        auto &modulator = modulators_[target][mod_type];
        if (modulator)
          modulator->On(sample_rate, patch.ModulationParams(target));
      }
    }
  }
  events.GeneratorOn(this);
}

void Generator::NoteRelease(SampleRate sample_rate, const GeneratorPatch &patch,
                            uint8_t note) {
  for (auto target : kModulationTargets) {
    auto mod_types = patch.ModTypesFor(target);
    for (int i = 0; i < Modulation::NumModulators; i++) {
      Modulation::Type mod_type = Modulation::Type(i);
      if (mod_types.test(mod_type)) {
        auto &modulator = modulators_[target][mod_type];
        if (modulator)
          modulator->Release(sample_rate, patch.ModulationParams(target));
      }
    }
  }
  events.GeneratorRelease(this);
}

void Generator::Reset() {
  for (const auto &target : kModulationTargets) {
    for (int i = 0; i < Modulation::NumModulators; i++) {
      auto &mod = modulators_[target][i];
      if (mod) mod->Reset();
    }
  }
}

void Generator::ConfigureModulators(const GeneratorPatch &patch) {
  for (const auto &target : kModulationTargets) {
    auto mod_types = patch.ModTypesFor(target);
    if (mod_types.test(Modulation::Envelope)) {
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
      modulators_[target][Modulation::Envelope] = std::move(envgen);
    }
    if (mod_types.test(Modulation::LFO)) {
      modulators_[target][Modulation::LFO] = std::make_unique<LFO>();
    }
  }
}

}  // namespace sidebands