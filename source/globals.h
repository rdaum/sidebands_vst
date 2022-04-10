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

constexpr const char *kRequestAnalysisBufferMessageID = "kRequestAnalysisBufferMessageID";
constexpr const char *kRequestAnalysisBufferGennnum = "kRequestAnalysisBufferGennnum";
constexpr const char *kRequestAnalysisBufferSampleRate = "kRequestAnalysisBufferSampleRate";
constexpr const char *kRequestAnalysisBufferSize = "kRequestAnalysisBufferSize";
constexpr const char *kRequestAnalysisBufferFreq = "kRequestAnalysisBufferFreq";

constexpr const char *kResponseAnalysisBufferMessageID = "kResponseAnalysisBufferMessageID";
constexpr const char *kResponseAnalysisBufferGennum = "kResponseAnalysisBufferGennum";
constexpr const char *kResponseAnalysisBufferSampleRate = "kResponseAnalysisBufferSampleRate";
constexpr const char *kResponseAnalysisBufferSize = "kResponseAnalysisBufferSize";
constexpr const char *kResponseAnalysisBufferFreq = "kResponseAnalysisBufferFreq";
constexpr const char *kResponseAnalysisBufferData = "kResponseAnalysisBufferData";

constexpr const char *kResponseSpectrumBufferMessageID = "kResponseSpectrumBufferMessageID";
constexpr const char *kResponseSpectrumBufferGennum = "kResponseSpectrumBufferGennum";
constexpr const char *kResponseSpectrumBufferSampleRate = "kResponseSpectrumBufferSampleRate";
constexpr const char *kResponseSpectrumBufferSize = "kResponseSpectrumBufferSize";
constexpr const char *kResponseSpectrumBufferFreq = "kResponseSpectrumBufferFreq";
constexpr const char *kResponseSpectrumBufferData = "kResponseSpectrumBufferData";

} // namespace sidebands