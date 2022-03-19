#include "adsr_envgen.h"

#include <glog/logging.h>

#include "globals.h"

namespace sidebands {

void ADSREnvelopeGenerator::Amplitudes(
    SampleRate sample_rate, OscBuffer &buffer, ParamValue velocity,
    const GeneratorPatch::ModParams *parameters) {
  std::lock_guard<std::mutex> stages_lock(stages_mutex_);
  const auto &ev = parameters->adsr_parameters;

  for (int i = 0; i < buffer.size(); i++) {
    buffer[i] = NextSample(sample_rate, ev);
  }

  // Apply velocity scaling.
  auto velocity_scale = ev.VS.getValue() * velocity + (1 - ev.VS.getValue());
  VmulInplace(buffer, velocity_scale);
}

void ADSREnvelopeGenerator::SetStage(off_t stage_number) {
  off_t old_stage = current_stage_;
  current_stage_ = stage_number;
  current_sample_index_ = 0;
  if (current_stage_ >= stages_.size())
    current_stage_ = 0;
  LOG(INFO) << "Advanced to stage: " << current_stage_ << " ("
            << stages_[current_stage_].name << ")"
            << " from: " << old_stage << " (" << stages_[old_stage].name
            << ") duration: " << stages_[current_stage_].duration
            << " samples; coefficent: " << stages_[current_stage_].coefficient
            << " current level: " << current_level_;
}

ParamValue ADSREnvelopeGenerator::NextSample(
    SampleRate sample_rate, const GeneratorPatch::ADSREnvelopeValues &ev) {
  // Off or sustain...
  if (current_stage_ == 0 || current_stage_ == sustain_stage_)
    return current_level_;

  auto c = stages_[current_stage_].coefficient;
  // If we've passed the duration of the current stage, advance.
  if (current_sample_index_ >= stages_[current_stage_].duration) {
    SetStage(current_stage_ + 1);
  }

  current_level_ *= c;
  current_sample_index_++;
  return current_level_;
}

off_t ADSREnvelopeGenerator::AddStage(double sample_rate,
                                      const std::string &name,
                                      double start_level, double end_level,
                                      double duration) {
  off_t idx = stages_.size();
  double duration_samples = (1-duration) * sample_rate;
  stages_.push_back(
      {name, start_level, end_level,
       EnvelopeRampCoefficient(start_level, end_level, duration_samples),
       duration_samples});
  return idx;
}

void ADSREnvelopeGenerator::On(SampleRate sample_rate,
                               const GeneratorPatch::ModParams *parameters) {
  std::lock_guard<std::mutex> stages_lock(stages_mutex_);
  stages_.clear();
  const auto &env = parameters->adsr_parameters;

  stages_ = {
      Stage{"OFF", minimum_level_, minimum_level_, 0, 0},
  };
  AddStage(sample_rate, "HT", minimum_level_, minimum_level_,
           env.HT.getValue());
  AddStage(sample_rate, "Attack", minimum_level_, env.AL.getValue(),
           env.AR.getValue());
  AddStage(sample_rate, "Decay1", env.AL.getValue(), env.DL1.getValue(),
           env.DR1.getValue());
  AddStage(sample_rate, "Decay2", env.DL1.getValue(), env.SL.getValue(),
           env.DR2.getValue());
  sustain_stage_ =
      AddStage(sample_rate, "SUSTAIN", env.SL.getValue(), env.SL.getValue(), 0);
  release_stage_ = AddStage(sample_rate, "Release1", env.SL.getValue(), env.RL1.getValue(),
           env.RR1.getValue());
  SetStage(1);
  current_level_ = minimum_level_;
}

void ADSREnvelopeGenerator::Release(
    SampleRate sample_rate, const GeneratorPatch::ModParams *parameters) {
  std::lock_guard<std::mutex> stages_lock(stages_mutex_);

  SetStage(release_stage_);
}

void ADSREnvelopeGenerator::Reset() {
  std::lock_guard<std::mutex> stages_lock(stages_mutex_);

  current_sample_index_ = 0;
  current_level_ = minimum_level_;
  current_stage_ = 0;
}

ModType ADSREnvelopeGenerator::mod_type() const {
  return ModType::ADSR_ENVELOPE;
}

bool ADSREnvelopeGenerator::Playing() const {
  std::lock_guard<std::mutex> stages_lock(stages_mutex_);
  return current_stage_ != 0;
}

} // namespace sidebands
