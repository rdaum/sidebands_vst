#pragma once

#include <cstdint>
#include <vstgui/lib/ccolor.h>

namespace sidebands {

// The number of generators (and drawbars) to configure.
constexpr int32_t kNumGenerators = 16;

constexpr size_t kSampleAccurateChunkSizeSamples = 8;

constexpr VSTGUI::CColor kBgGrey(219, 219, 219);

} // namespace sidebands