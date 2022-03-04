#include "oscillator.h"

#include <numbers>
#include <valarray>

namespace sidebands {


void Oscillator::Perform(uint32_t sample_rate, OscBuffer &buffer,
                         OscParam &note_freq, OscParam &C,
                         OscParam &M, OscParam &R, OscParam &S, OscParam &K) {
  auto buffer_size = buffer.size();

  // Accumulate the time multiplier based on current phase.
  OscParam T(buffer_size);
  for (int i = 0; i < buffer_size; i++) {
    phase_++;
    T[i] = (phase_ / sample_rate);
  }

  auto freq = note_freq * C;
  auto omega_c = freq * std::numbers::pi * 2.0;
  auto omega_m = (M * freq) * std::numbers::pi * 2.0;
  auto omega_c_t = omega_c *  T;
  auto omega_m_t = omega_m * T;

  // modified
//  buffer = exp(K * cos(omega_m_t)) * cos(omega_c_t);

  // modified with S/R
  buffer =
      exp(R * K * cos(omega_m_t)) * cos(omega_c_t + S * K * sin(omega_m_t));
}

} // namespace sidebands