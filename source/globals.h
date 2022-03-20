#pragma once

#include <bitset>
#include <cstddef>
#include <pluginterfaces/vst/ivstmessage.h>

#include "constants.h"
#include "tags.h"

namespace sidebands {

// Calculate exponential ramping coefficient for envelope stages.
double EnvelopeRampCoefficient(double start_level, double end_level,
                               size_t length_in_samples);

// Used to signal various aspects of the currently playing state back to the
// UI thread, so it can do some fancy rendering.
struct PlayerState {
  uint32_t active_voices;
  struct VoiceState {
    double level;
    std::bitset<kNumGenerators> active_generators;
    struct GeneratorState {
      struct ModulationState {
        TargetTag target;
        off_t current_envelope_segment;
        double lfo_level;
      };
      ModulationState modulation_states[NUM_TARGETS];
    };
    GeneratorState generator_states[kNumGenerators];
  };
  VoiceState voice_states[kNumVoices];
};

constexpr const char *kAttrPlayerStateMessageID = "kAttrPlayerStateMessageID";
constexpr const char *kAttrPlayerStateAttr = "kAttrPlayerStateAttr";

Steinberg::tresult
SetPlayerStateAttributes(Steinberg::Vst::IAttributeList *attributes,
                         const PlayerState &player_state);
Steinberg::tresult
GetPlayerStateAttributes(Steinberg::Vst::IAttributeList *attributes,
                         PlayerState *player_state);

} // namespace sidebands