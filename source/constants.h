#pragma once

#include <vstgui/lib/ccolor.h>

#include <cstdint>

namespace sidebands {

// The number of generators (and drawbars) to configure.
constexpr int32_t kNumGenerators = 8;

// Minimum 8.
constexpr size_t kSampleAccurateChunkSizeSamples = 128;

constexpr VSTGUI::CColor kBgGrey(219, 219, 219);

}  // namespace sidebands