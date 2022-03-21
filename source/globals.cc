#include "globals.h"

#include <numeric>
#include <cmath>

namespace sidebands {

double EnvelopeRampCoefficient(double start_level, double end_level,
                               size_t length_in_samples) {
  return 1.0 + (std::log(end_level) - std::log(start_level)) /
                   ((double)length_in_samples);
}

} // namespace sidebands