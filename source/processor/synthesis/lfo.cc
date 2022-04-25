#include "processor/synthesis/lfo.h"

#include <numbers>

namespace sidebands {

namespace {
constexpr double kTwoPi = 2.0 * std::numbers::pi;
}  // namespace

void LFO::On(SampleRate sample_rate,
             const GeneratorPatch::ModParams *parameters) {
  phase_ = 0.0;
  playing_ = true;
}

void LFO::Release(SampleRate sample_rate,
                  const GeneratorPatch::ModParams *parameters) {
  Reset();
}

void LFO::Reset() {
  phase_ = 0.0;
  playing_ = false;
}

void LFO::Amplitudes(SampleRate sample_rate, OscBuffer &buffer,
                     ParamValue velocity,
                     const GeneratorPatch::ModParams *parameters) {
  const auto buffer_size = buffer.size();
  const auto &lfo_values = parameters->lfo_parameters;
  const auto freq = lfo_values.frequency.getValue();
  double phase_increment = kTwoPi * freq / sample_rate;

  OscBuffer phases(buffer_size);

  // TODO: A way to do this non-incrementally, bulky bulk with SIMD?
  for (int i = 0; i < buffer_size; i++) {
    phase_ += phase_increment;
    phases[i] = phase_;
    if (phase_ >= kTwoPi) phase_ -= kTwoPi;
  }

  buffer = kLFOTypes[off_t(lfo_values.type.getValue())] == LFOType::SIN
               ? Vsin(phases)
               : Vcos(phases);

  last_level_ = buffer[0];

  auto velocity_scale = (lfo_values.velocity_sensivity.getValue() * velocity) +
                        (1 - lfo_values.velocity_sensivity.getValue());
  auto amplitude = lfo_values.amplitude.getValue() * velocity_scale;

  VmulInplace(buffer, amplitude);
}

bool LFO::Playing() const { return playing_; }

Modulation::Type LFO::mod_type() const { return Modulation::LFO; }

}  // namespace sidebands