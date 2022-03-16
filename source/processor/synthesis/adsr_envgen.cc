#include "adsr_envgen.h"

#include <glog/logging.h>

namespace sidebands {

double Coefficient(double start_level, double end_level,
                   size_t length_in_samples) {
  return 1.0 + (std::log(end_level) - std::log(start_level)) /
                   ((ParamValue)length_in_samples);
}
constexpr const char *kStageLabels[]{"OFF", "ATTACK", "DECAY", "SUSTAIN",
                                     "RELEASE"};

// originally based on
// http://www.martin-finke.de/blog/articles/audio-plugins-011-envelopes/
// TODO: too much branching in loops, need to be able to set levels (not just
// rates), more stages, aftertouch

ParamValue
ADSREnvelopeGenerator::NextSample(SampleRate sample_rate, ParamValue velocity,
                                  const GeneratorPatch::ModTarget *parameters) {
  if (stage_ == ENVELOPE_STAGE_OFF)
    return current_level_;

  const auto &ev = parameters->adsr_parameters;

  // Vel sense of 1 means respond fully to velocity, 0 not velocity sensitive.
  // Inbetween we scale.
  auto velocity_scale = (ev.VS.getValue() * velocity) + (1 - ev.VS.getValue());

  if (stage_ == ENVELOPE_STAGE_SUSTAIN)
    return current_level_ * velocity_scale;

  if (current_sample_index_ >= next_stage_sample_index_) {
    auto next_stage =
        static_cast<EnvelopeStage>((stage_ + 1) % kNumEnvelopeStages);
    EnterStage(sample_rate, next_stage, ev);
  }

  current_level_ *= coefficient_;
  current_sample_index_++;
  return current_level_ * velocity_scale;
}

void ADSREnvelopeGenerator::EnterStage(
    SampleRate sample_rate, EnvelopeStage new_stage,
    const GeneratorPatch::ADSREnvelopeValues &envelope) {
  const ParamValue stage_rates_[]{
      0.0, envelope.A_R.getValue(), envelope.D_R.getValue(),
      envelope.S_L.getValue(), envelope.R_R.getValue()};
  stage_ = new_stage;
  current_sample_index_ = 0;
  if (stage_ != ENVELOPE_STAGE_OFF && stage_ != ENVELOPE_STAGE_SUSTAIN) {
    next_stage_sample_index_ = stage_rates_[stage_] * sample_rate;
  }
  switch (new_stage) {
  case ENVELOPE_STAGE_OFF:
    Reset();
    break;
  case ENVELOPE_STAGE_ATTACK:
    current_level_ = minimum_level_;
    coefficient_ = Coefficient(current_level_, envelope.A_L.getValue(),
                               next_stage_sample_index_);
    break;
  case ENVELOPE_STAGE_DECAY:
    current_level_ = envelope.A_L.getValue();
    coefficient_ = Coefficient(
        current_level_,
        std::fmax(stage_rates_[ENVELOPE_STAGE_SUSTAIN], minimum_level_),
        next_stage_sample_index_);
    break;
  case ENVELOPE_STAGE_SUSTAIN:
    current_level_ = stage_rates_[ENVELOPE_STAGE_SUSTAIN];
    coefficient_ = 1.0f;
    break;
  case ENVELOPE_STAGE_RELEASE:
    // We could go from ATTACK/DECAY to RELEASE,
    // so we're not changing currentLevel here.
    coefficient_ =
        Coefficient(current_level_, minimum_level_, next_stage_sample_index_);
    break;
  default:
    break;
  }
}

void ADSREnvelopeGenerator::On(SampleRate sample_rate,
                               const GeneratorPatch::ModTarget *parameters) {
  EnterStage(sample_rate, ENVELOPE_STAGE_ATTACK, parameters->adsr_parameters);
}

void ADSREnvelopeGenerator::Release(
    SampleRate sample_rate, const GeneratorPatch::ModTarget *parameters) {
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
