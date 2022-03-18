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

void ADSREnvelopeGenerator::SetStage(off_t stage_number) {
  off_t old_stage = stage_idx_;
  stage_idx_ = stage_number;
  current_sample_index_ = 0;
  if (stage_idx_ >= stages_.size())
    stage_idx_ = 0;
  LOG(INFO) << "Advanced to stage: " << stage_idx_ << " ("
            << stages_[stage_idx_].name << ")"
            << " from: " << old_stage << " (" << stages_[old_stage].name
            << ") duration: " << stages_[stage_idx_].duration
            << " samples; coefficent: " << stages_[stage_idx_].coefficient;
}

ParamValue ADSREnvelopeGenerator::NextSample(
    SampleRate sample_rate, const GeneratorPatch::ADSREnvelopeValues &ev) {
  // Off or sustain...
  if (stages_[stage_idx_].type == Stage::Type::OFF ||
      stages_[stage_idx_].type == Stage::Type::LEVEL)
    return current_level_;

  // If we've passed the duration of the current stage, advance.
  if (stages_[stage_idx_].type == Stage::Type::RATE &&
      current_sample_index_ >= stages_[stage_idx_].duration) {
    SetStage(stage_idx_ + 1);
  }

  current_level_ *= stages_[stage_idx_].coefficient;
  current_sample_index_++;
  return current_level_;
}

off_t ADSREnvelopeGenerator::AddStage(double sample_rate,
                                      const std::string &name, Stage::Type type,
                                      double start_level, double end_level,
                                      double duration) {
  off_t idx = stages_.size();
  double duration_samples = duration * sample_rate;
  stages_.push_back(
      {name, type, start_level, end_level,
       EnvelopeRampCoefficient(start_level, end_level, duration_samples),
       duration_samples});
  return idx;
}

void ADSREnvelopeGenerator::On(SampleRate sample_rate,
                               const GeneratorPatch::ModParams *parameters) {
  stages_.clear();
  const auto &env = parameters->adsr_parameters;

  stages_ = {
      Stage{"OFF", Stage::Type::OFF, minimum_level_, minimum_level_, 0, 0},
  };
  AddStage(sample_rate, "Attack", Stage::Type::RATE, minimum_level_,
           env.A_L.getValue(), env.A_R.getValue());
  AddStage(sample_rate, "Decay", Stage::Type::RATE, env.A_L.getValue(),
           env.S_L.getValue(), env.D_R.getValue());
  AddStage(sample_rate, "Sustain", Stage::Type::LEVEL, env.S_L.getValue(),
           env.S_L.getValue(), 0);
  release_stage_idx_ =
      AddStage(sample_rate, "Release", Stage::Type::RATE, env.S_L.getValue(),
               minimum_level_, env.R_R.getValue());
  SetStage(1);
  current_level_ = minimum_level_;
}

void ADSREnvelopeGenerator::Release(
    SampleRate sample_rate, const GeneratorPatch::ModParams *parameters) {
  SetStage(release_stage_idx_);
}

void ADSREnvelopeGenerator::Reset() {
  current_sample_index_ = 0;
  current_level_ = minimum_level_;

  SetStage(0);
}

ModType ADSREnvelopeGenerator::mod_type() const {
  return ModType::ADSR_ENVELOPE;
}

bool ADSREnvelopeGenerator::Playing() const { return stage_idx_ != 0; }

} // namespace sidebands
