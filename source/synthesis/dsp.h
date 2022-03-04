#pragma once

#include <complex>
#include <valarray>

namespace sidebands {

using OscBuffer = std::valarray<double>;
using OscParam = std::valarray<double>;

using VDArray = std::valarray<double>;

void DCBlock(OscBuffer &buffer, const OscParam &index /* between 0.9 and 1 */);

void linspace(VDArray &linspaced, double start, double end, size_t num);

}  // namespace sidebands