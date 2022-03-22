#pragma once

#include "dsp/oscbuffer.h"

namespace sidebands {

struct Integrator {
  explicit Integrator(double b = 0.998) : b1_(b) {}
  void Filter(OscBuffer &buf);
  double b1_ = 0.0;
  double y1_ = 0.f;
};

}  // namespace sidebands