#include "oscillator.h"

#include <numbers>

namespace sidebands {

namespace {

constexpr double kPi2 = std::numbers::pi * 2.0;

}  // namespace

OscParams::OscParams(size_t buffer_size)
    : note_freq(buffer_size),
      C(buffer_size),
      M(buffer_size),
      R(buffer_size),
      S(buffer_size),
      K(buffer_size) {}

void ModFMOscillator::Perform(Steinberg::Vst::SampleRate sample_rate,
                              OscBuffer &buffer, OscParams &params) {
  auto buffer_size = buffer.size();

  // Accumulate the time multiplier based on current phase.
  OscParam T(buffer_size);
  linspace(T, phase_ / sample_rate, (phase_ + buffer_size) / sample_rate,
           buffer_size);
  phase_ += buffer_size;

  auto freq = Vmul(params.note_freq, params.C);
  auto omega_c = Vmul(freq, kPi2);
  auto omega_m = Vmul(omega_c, params.M);

  VmulInplace(omega_c, T);
  VmulInplace(omega_m, T);

  buffer =
      Vmul(Vexp(Vmul(Vmul(params.R, params.K), Vcos(omega_m))),
           Vcos(Vadd(omega_c, Vmul(Vmul(params.S, params.K), Vsin(omega_m)))));

  // normalize for K by dividing out exp of K
  VdivInplace(buffer, Vexp(params.K));

  /*
  buffer =
      (exp(R * K * cos(omega_m)) * cos(omega_c + S * K * sin(omega_m))) /
      exp(K) /* normalize for K by dividing out exp of K */
  ;
}

AnalogOscillator::AnalogOscillator() {}

void AnalogOscillator::Perform(Steinberg::Vst::SampleRate sample_rate,
                               OscBuffer &buffer, OscParams &params) {
  auto buffer_size = buffer.size();

  // Accumulate the time multiplier based on current phase.
  OscParam T(buffer_size);
  linspace(T, phase_ / sample_rate, (phase_ + buffer_size) / sample_rate,
           buffer_size);
  phase_ += buffer_size;

  auto freq = Vmul(params.note_freq, params.C);
  auto omega_c = Vmul(freq, kPi2);
  auto omega_m = Vmul(omega_c, params.M);

  VmulInplace(omega_c, T);
  VmulInplace(omega_m, T);

  params.K *= 10;

  // We start by producing a pulse train using a variant of ModFM.
  // Modulation index controls width of pulse.
  buffer = exp(params.K * cos(omega_m) - params.K) * cos(omega_c);

  // To go to saw from pulse, we need to integrate and then dc block as per the
  // paper.
  int_.Filter(buffer);
  dc_.Filter(buffer);
}

std::unique_ptr<IOscillator> MakeOscillator(GeneratorPatch::OscType type) {
  switch (type) {
    case GeneratorPatch::OscType::ANALOG:
      return std::make_unique<AnalogOscillator>();
    case GeneratorPatch::OscType::MOD_FM:
    default:
      return std::make_unique<ModFMOscillator>();
  }
}

}  // namespace sidebands