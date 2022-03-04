#include "synthesis/dsp.h"

namespace sidebands {

void DCBlock(OscBuffer &X, const OscParam &gain) {
  OscBuffer Y(X.size());
  auto input = X[0];
  for (int i = 0; i < X.size(); i++) {
    auto sample = X[i];
    Y[i] = X[i] - input + (gain[i] * Y[i]);
    input = X[i];
  }
  X = Y;
}

void linspace(VDArray &linspaced, double start, double end, size_t num) {
  if (num == 0) {
    return;
  }
  if (num == 1) {
    linspaced[0] = start;
    return;
  }

  double delta = (end - start) / (num - 1);

  int i = 0;
  for (i = 0; i < num; i++) {
    linspaced[i] = (start + delta * i);
  }
  linspaced[i++] = end; // I want to ensure that start and end
                        // are exactly the same as the input
  return;
}




} // namespace sidebands