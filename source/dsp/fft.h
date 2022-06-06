#pragma once

#include <complex>
#include <valarray>

using Complex = std::complex<double>;
using ComplexBuffer = std::valarray<Complex>;

namespace sidebands {

ComplexBuffer ScalarToComplex(const double *scalar_buffer, size_t buffer_size);
std::valarray<double> ComplexToScalar(const ComplexBuffer &b);

void FFT(ComplexBuffer &x);
void HanningWindow(ComplexBuffer &buf);
void HammingWindow(ComplexBuffer &buf);
void BlackingWindow(ComplexBuffer &buf);

}  // namespace sidebands