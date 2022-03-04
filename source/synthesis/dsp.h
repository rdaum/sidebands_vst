#pragma once

#include <complex>
#include <valarray>

namespace sidebands {

using OscBuffer = std::valarray<std::complex<double>>;
using OscParam = std::valarray<std::complex<double>>;

using VDArray = std::valarray<double>;

void DCBlock(OscBuffer &buffer, const OscParam &index /* between 0.9 and 1 */);

void linspace(VDArray &linspaced, double start, double end, size_t num);

}  // namespace sidebands