#pragma once

#include <bitset>
#include <cstddef>
#include <pluginterfaces/vst/ivstmessage.h>

namespace sidebands {

// Calculate exponential ramping coefficient for envelope stages.
double EnvelopeRampCoefficient(double start_level, double end_level,
                               size_t length_in_samples);

constexpr const char *kEnvelopeStageMessageID = "kEnvelopeStageMessageID";
constexpr const char *kEnvelopeStageNoteIDAttr = "kEnvelopeStageNoteIDAttr";
constexpr const char *kEnvelopeStageGennumAttr = "kEnvelopeStageGennumAttr";
constexpr const char *kEnvelopeStageTargetAttr = "kEnvelopeStageTargetAttr";
constexpr const char *kEnvelopeStageStageAttr = "kEnvelopeStageStageAttr";

} // namespace sidebands