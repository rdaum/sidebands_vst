#pragma once

#include <complex>
#include <cstddef>
#include <cstdint>
#include <valarray>

#include "processor/synthesis/dsp.h"
#include "processor/patch_processor.h"

namespace sidebands {

using Steinberg::Vst::ParamValue;

class Oscillator {
 public:
  void Perform(Steinberg::Vst::SampleRate sample_rate, OscBuffer &buffer, OscParam &note_freq,
               OscParam &C, OscParam &M, OscParam &R, OscParam &S, OscParam &K);

  void Reset() { phase_ = 0.0f; }

 private:
  double phase_ = 0.0f;
};

}  // namespace sidebands