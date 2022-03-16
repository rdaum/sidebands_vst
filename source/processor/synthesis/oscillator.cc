#include "oscillator.h"

#include <numbers>

namespace sidebands {

namespace {

constexpr double kPi2 = std::numbers::pi * 2.0;

} // namespace

void Oscillator::Perform(Steinberg::Vst::SampleRate sample_rate,
                         OscBuffer &buffer, OscParam &note_freq, OscParam &C,
                         OscParam &M, OscParam &R, OscParam &S, OscParam &K) {
  auto buffer_size = buffer.size();

  // Accumulate the time multiplier based on current phase.
  OscParam T(buffer_size);
  linspace(T, phase_ / sample_rate, (phase_ + buffer_size) / sample_rate,
           buffer_size);
  phase_ += buffer_size;

  auto freq = Vmul(note_freq, C);
  auto omega_c = Vmul(freq, kPi2);
  auto omega_m = Vmul(omega_c, M);

  VmulInplace(omega_c, T);
  VmulInplace(omega_m, T);

  buffer = Vmul(Vexp(Vmul(Vmul(R, K), Vcos(omega_m))),
                Vcos(Vadd(omega_c, Vmul(Vmul(S, K), Vsin(omega_m)))));

  // normalize for K by dividing out exp of K
  VdivInplace(buffer, Vexp(K));

  /*
  buffer =
      (exp(R * K * cos(omega_m)) * cos(omega_c + S * K * sin(omega_m))) /
      exp(K) /* normalize for K by dividing out exp of K */
  ;
}

} // namespace sidebands