#include "dsp/fft.h"

#include <numbers>

#include "dsp/oscbuffer.h"

namespace sidebands {

ComplexBuffer ScalarToComplex(const double *scalar_buffer, size_t buffer_size) {
  ComplexBuffer carray(buffer_size);
  for (int i = 0; i < buffer_size; i++) {
    carray[i] = scalar_buffer[i];
  }
  return std::move(carray);
}

std::valarray<double> ComplexToScalar(const ComplexBuffer &b) {
  std::valarray<double> sarray(b.size());
  for (int i = 0; i < b.size(); i++) {
    sarray[i] = b[i].real();
  }
  return std::move(sarray);
}

// Cooley–Tukey FFT (in-place, divide-and-conquer)
// Higher memory requirements and redundancy although more intuitive
void FFT(ComplexBuffer &x) {
  // DFT
  unsigned int N = x.size(), k = N, n;
  double thetaT = 3.14159265358979323846264338328L / N;
  Complex phiT = Complex(cos(thetaT), -sin(thetaT)), T;
  while (k > 1) {
    n = k;
    k >>= 1;
    phiT = phiT * phiT;
    T = 1.0L;
    for (unsigned int l = 0; l < k; l++) {
      for (unsigned int a = l; a < N; a += n) {
        unsigned int b = a + k;
        Complex t = x[a] - x[b];
        x[a] += x[b];
        x[b] = t * T;
      }
      T *= phiT;
    }
  }
  // Decimate
  auto m = (unsigned int)log2(N);
  for (unsigned int a = 0; a < N; a++) {
    unsigned int b = a;
    // Reverse bits
    b = (((b & 0xaaaaaaaa) >> 1) | ((b & 0x55555555) << 1));
    b = (((b & 0xcccccccc) >> 2) | ((b & 0x33333333) << 2));
    b = (((b & 0xf0f0f0f0) >> 4) | ((b & 0x0f0f0f0f) << 4));
    b = (((b & 0xff00ff00) >> 8) | ((b & 0x00ff00ff) << 8));
    b = ((b >> 16) | (b << 16)) >> (32 - m);
    if (b > a) {
      Complex t = x[a];
      x[a] = x[b];
      x[b] = t;
    }
  }
  //// Normalize (This section make it not working correctly)
  // Complex f = 1.0 / sqrt(N);
  // for (unsigned int i = 0; i < N; i++)
  //	x[i] *= f;
}

void HanningWindow(ComplexBuffer &buf) {
  auto samples_minus_1 = buf.size() - 1;
  for (int i = 0; i < buf.size(); i++) {
    auto multiplier =
        0.5 * (1 - cos(2 * std::numbers::pi * i / samples_minus_1));
    buf[i] = multiplier * buf[i];
  }
}

void HammingWindow(ComplexBuffer &buf) {
  double samples_minus_1 = buf.size() - 1;
  for (int i = 0; i < buf.size(); i++) {
    auto multiplier =
        0.54 -
        (0.46 * std::cos(2 * std::numbers::pi * (double(i) / samples_minus_1)));
    buf[i] = multiplier * buf[i];
  }
}

void BlackingWindow(ComplexBuffer &buf) {
  double samples_minus_1 = buf.size() - 1;

  for (int i = 0; i < buf.size(); i++) {
    auto multiplier =
        0.42 -
        (0.5 * cos(2. * std::numbers::pi * (double(i) / samples_minus_1))) +
        (0.08 * cos(4. * std::numbers::pi * (double(i) / samples_minus_1)));
    buf[i] *= multiplier;
  }
}
}  // namespace sidebands
