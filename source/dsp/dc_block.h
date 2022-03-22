#pragma once

#include "dsp/oscbuffer.h"

namespace sidebands {

struct DCBlock {
  explicit DCBlock(double b = 0.999) : b1_(b) {}
  void Filter(OscBuffer &buf);
  double b1_;
  double x1_ = 0.0;
  double y1_ = 0.0;
};

struct DCBlock2 {
  explicit DCBlock2(int order = 128);

  void Filter(OscBuffer &in);

  OscBuffer delay1_;
  OscBuffer iirdelay1_;
  OscBuffer iirdelay2_;
  OscBuffer iirdelay3_;
  OscBuffer iirdelay4_;
  double ydels_[4];
  int32_t dp1_, dp2_;
  double scaler_;
};
}  // namespace sidebands
