#include "adsr_envgen.h"

#include <glog/logging.h>

#include "globals.h"

namespace sidebands {

void ADSREnvelopeGenerator::Amplitudes(
    SampleRate sample_rate, OscBuffer &buffer, ParamValue velocity,
    const GeneratorPatch::ModParams *parameters) {
  const auto &ev = parameters->adsr_parameters;

  for (int i = 0; i < buffer.size(); i++) {
    buffer[i] = NextSample(sample_rate, ev);
  }

  // Apply velocity scaling.
  auto velocity_scale = ev.VS.getValue() * velocity + (1 - ev.VS.getValue());
  VmulInplace(buffer, velocity_scale);
}

ParamValue ADSREnvelopeGenerator::NextSample(
    SampleRate sample_rate, const GeneratorPatch::ADSREnvelopeValues &ev) {
  if (stage_ == ENVELOPE_STAGE_OFF)
    return current_level_;

  if (stage_ == ENVELOPE_STAGE_SUSTAIN)
    return current_level_;

  if (current_sample_index_ >= next_stage_sample_index_) {
    auto next_stage =
        static_cast<EnvelopeStage>((stage_ + 1) % kNumEnvelopeStages);
    EnterStage(sample_rate, next_stage, ev);
  }

  current_level_ *= coefficient_;
  current_sample_index_++;
  return current_level_;
}

void ADSREnvelopeGenerator::EnterStage(
    SampleRate sample_rate, EnvelopeStage new_stage,
    const GeneratorPatch::ADSREnvelopeValues &envelope) {
  const ParamValue stage_rates[]{
      0.0, envelope.A_R.getValue(), envelope.D_R.getValue(),
      envelope.S_L.getValue(), envelope.R_R.getValue()};
  stage_ = new_stage;
  current_sample_index_ = 0;
  if (stage_ != ENVELOPE_STAGE_OFF && stage_ != ENVELOPE_STAGE_SUSTAIN) {
    next_stage_sample_index_ = stage_rates[stage_] * sample_rate;
  }
  switch (new_stage) {
  case ENVELOPE_STAGE_OFF:
    Reset();
    break;
  case ENVELOPE_STAGE_ATTACK:
    current_level_ = minimum_level_;
    coefficient_ = EnvelopeRampCoefficient(
        current_level_, envelope.A_L.getValue(), next_stage_sample_index_);
    break;
  case ENVELOPE_STAGE_DECAY:
    current_level_ = envelope.A_L.getValue();
    coefficient_ = EnvelopeRampCoefficient(
        current_level_,
        std::max(stage_rates[ENVELOPE_STAGE_SUSTAIN], minimum_level_),
        next_stage_sample_index_);
    break;
  case ENVELOPE_STAGE_SUSTAIN:
    current_level_ = stage_rates[ENVELOPE_STAGE_SUSTAIN];
    coefficient_ = 1.0f;
    break;
  case ENVELOPE_STAGE_RELEASE:
    coefficient_ = EnvelopeRampCoefficient(current_level_, minimum_level_,
                                           next_stage_sample_index_);
    break;
  default:
    break;
  }
}

void ADSREnvelopeGenerator::On(SampleRate sample_rate,
                               const GeneratorPatch::ModParams *parameters) {
  EnterStage(sample_rate, ENVELOPE_STAGE_ATTACK, parameters->adsr_parameters);
}

void ADSREnvelopeGenerator::Release(
    SampleRate sample_rate, const GeneratorPatch::ModParams *parameters) {
  EnterStage(sample_rate, ENVELOPE_STAGE_RELEASE, parameters->adsr_parameters);
}

void ADSREnvelopeGenerator::Reset() {
  current_sample_index_ = 0;
  current_level_ = 0.0;
  coefficient_ = 1.0f;
  stage_ = ENVELOPE_STAGE_OFF;
}

ModType ADSREnvelopeGenerator::mod_type() const {
  return ModType::ADSR_ENVELOPE;
}

} // namespace sidebands
