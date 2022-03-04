#pragma once

#include <complex>
#include <cstddef>
#include <cstdint>
#include <valarray>

#include "synthesis/dsp.h"
#include "synthesis/patch.h"

namespace sidebands {

using Steinberg::Vst::ParamValue;

class Oscillator {
public:
  using OscBuffer = std::valarray<std::complex<double>>;
  using OscParam = std::valarray<std::complex<double>>;

  void Perform(uint32_t sample_rate, OscBuffer &buffer, OscParam &note_freq,
               OscParam &C, OscParam &M, OscParam &R, OscParam &S,
               OscParam &K);

  void Reset() { phase_ = 0.0f; }

private:
  ParamValue phase_ = 0.0f;
};

} // namespace sidebands