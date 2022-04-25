#pragma once

#include <pluginterfaces/vst/ivstmessage.h>

#include <bitset>
#include <cstddef>

namespace sidebands {

// Calculate exponential ramping coefficient for envelope stages.
double EnvelopeRampCoefficient(double start_level, double end_level,
                               size_t length_in_samples);

constexpr const char *kRequestAnalysisBufferMessageID =
    "kRequestAnalysisBufferMessageID";
constexpr const char *kResponseAnalysisBufferMessageID =
    "kResponseAnalysisBufferMessageID";
constexpr const char *kRequestSpectrumBufferMessageID =
    "kRequestSpectrumBufferMessageID";
constexpr const char *kResponseSpectrumBufferMessageID =
    "kResponseSpectrumBufferMessageID";
constexpr const char *kEnvelopeStageMessageID = "kEnvelopeStageMessageID";

constexpr const char *kNoteIdAttr = "noteId";
constexpr const char *kTargetAttr = "target";
constexpr const char *kEnvelopeStageAttr = "envelopeStage";
constexpr const char *kGennumAttr = "gennum";
constexpr const char *kSampleRateAttr = "sampleRate";
constexpr const char *kBufferSizeAttr = "bufferSize";
constexpr const char *kBufferDataAttr = "bufferData";
constexpr const char *kFreqAttr = "frequency";

}  // namespace sidebands