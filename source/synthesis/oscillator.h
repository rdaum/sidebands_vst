#pragma once

#include <complex>
#include <cstddef>
#include <cstdint>
#include <pluginterfaces/vst/vsttypes.h>

#include "patch.h"

namespace sidebands {

using Steinberg::Vst::ParamValue;

class Oscillator {
public:
  void Perform(size_t buffer_size, uint16_t sample_rate,
               std::complex<double> buffer[], const double freq,
               const double level_a[], const double level_c[],
               const double level_m[], const double level_r[],
               const double level_s[], const double level_k[]);

  void Reset() { x_ = 0.0f; }

private:
  ParamValue x_ = 0.0f;
};

} // namespace sidebands