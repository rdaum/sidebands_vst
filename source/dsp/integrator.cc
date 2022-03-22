#include "dsp/integrator.h"

namespace sidebands {

void Integrator::Filter(OscBuffer &buf) {
  double b1 = b1_;
  double y1 = y1_;

  if (b1 == 1.f) {
    for (int i = 0; i < buf.size(); i++) {
      double y0 = buf[i];
      buf[i] = y1 = y0 + y1;
    }
  } else if (b1 == 0.f) {
    for (int i = 0; i < buf.size(); i++) {
      double y0 = buf[i];
      buf[i] = y1 = y0 + b1 * y1;
    }
  } else {
    for (int i = 0; i < buf.size(); i++) {
      double y0 = buf[i];
      buf[i] = y1 = y0 + b1 * y1;
    }
  }

  y1_ = zapgremlins(y1);
}
}  // namespace sidebands