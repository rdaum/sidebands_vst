#pragma once

#include <cstddef>

namespace sidebands {

// Calculate exponential ramping coefficient for envelope stages.
double EnvelopeRampCoefficient(double start_level, double end_level,
                               size_t length_in_samples);

} // namespace sidebands