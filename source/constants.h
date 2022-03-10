#pragma once

#include <vstgui/lib/ccolor.h>

#include <cstdint>

namespace sidebands {

// The number of generators (and drawbars) to configure.
constexpr int32_t kNumGenerators = 8;

// Minimum 8.
constexpr size_t kSampleAccurateChunkSizeSamples = 128;

enum class LFOType { SIN, COS };
constexpr LFOType kLFOTypes[]{LFOType::SIN, LFOType::COS};
constexpr int kNumLFOTypes = sizeof(kLFOTypes) / sizeof(LFOType);

enum class ModType { NONE, ENVELOPE, LFO };
constexpr ModType kModTypes[]{ModType::NONE, ModType::ENVELOPE,
                                     ModType::LFO};
constexpr int kNumModTypes = sizeof(kModTypes) / sizeof(ModType);

} // namespace sidebands