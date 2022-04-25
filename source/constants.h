#pragma once

#include <cstdint>

namespace sidebands {

// The number of generators to configure.
constexpr int32_t kNumGenerators = 8;

constexpr int kNumVoices = 8;  // Must be power of 2.

// Minimum 8.
constexpr size_t kSampleAccurateChunkSizeSamples = 32;

enum class LFOType { SIN, COS };
constexpr LFOType kLFOTypes[]{LFOType::SIN, LFOType::COS};
constexpr int kNumLFOTypes = sizeof(kLFOTypes) / sizeof(LFOType);

namespace Modulation {
enum Type { Envelope, LFO, NumModulators };
}  // namespace Modulation

}  // namespace sidebands