#include "synthesis/lfo.h"

#include <numbers>

namespace sidebands {

void LFO::On(SampleRate sample_rate,
             const GeneratorPatch::ModulationParameters &parameters) {
  phase_ = 0.0;
  playing_ = true;
}

void LFO::Release(SampleRate sample_rate,
                  const GeneratorPatch::ModulationParameters &parameters) {
  Reset();
}

void LFO::Reset() {
  phase_ = 0.0;
  playing_ = false;
}

ParamValue
LFO::NextSample(SampleRate sample_rate, ParamValue velocity,
                const GeneratorPatch::ModulationParameters &parameters) {
  auto &lfo_values = std::get<GeneratorPatch::LFOValues>(parameters);
  phase_ +=
      2.0 * std::numbers::pi * lfo_values.frequency.getValue() / sample_rate;
  while (phase_ >= 2.0 * std::numbers::pi)
    phase_ -= 2.0 * std::numbers::pi;

  auto velocity_scale = (lfo_values.velocity_sensivity.getValue() * velocity) +
                        (1 - lfo_values.velocity_sensivity.getValue());
  auto amplitude = lfo_values.amplitude.getValue() * velocity_scale;
  return (GeneratorPatch::kLFOTypes[off_t(lfo_values.type.getValue())] ==
                  GeneratorPatch::LFOType::SIN
              ? std::sin(phase_)
              : std::cos(phase_)) *
         amplitude;
}

bool LFO::Playing() const { return playing_; }

GeneratorPatch::ModType LFO::mod_type() const {
  return GeneratorPatch::ModType::LFO;
}

} // namespace sidebands