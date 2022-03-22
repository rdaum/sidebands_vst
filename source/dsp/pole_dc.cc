#include "dsp/pole_dc.h"

#include <glog/logging.h>

#include "dsp/oscbuffer.h"

namespace sidebands {

PoleZero ::PoleZero() {
  gain_ = 1.0;

  // Default setting for pass-through.
  b_.resize(2, 0.0);
  a_.resize(2, 0.0);
  b_[0] = 1.0;
  a_[0] = 1.0;
  inputs_.resize(2, 0.0);
  outputs_.resize(2, 0.0);
  lastFrame_.resize(1, 0.0);
}

void PoleZero ::setCoefficients(double b0, double b1, double a1,
                                bool clearState) {
  if (std::abs(a1) >= 1.0) {
    LOG(ERROR) << "PoleZero::setCoefficients: a1 argument (" << a1
               << ") should be less than 1.0!";
    return;
  }

  b_[0] = b0;
  b_[1] = b1;
  a_[1] = a1;

  if (clearState)
    this->clear();
}

void PoleZero ::setAllpass(double coefficient) {
  if (std::abs(coefficient) >= 1.0) {
    LOG(ERROR) << "PoleZero::setAllpass: argument (" << coefficient
               << ") makes filter unstable!";
    return;
  }

  b_[0] = coefficient;
  b_[1] = 1.0;
  a_[0] = 1.0; // just in case
  a_[1] = coefficient;
}

void PoleZero ::setBlockZero(double thePole) {
  if (std::abs(thePole) >= 1.0) {
    LOG(ERROR) << "PoleZero::setBlockZero: argument (" << thePole
               << ") makes filter unstable!";
    return;
  }

  b_[0] = 1.0;
  b_[1] = -1.0;
  a_[0] = 1.0; // just in case
  a_[1] = -thePole;
}

double PoleZero ::tick(double input) {
  inputs_[0] = gain_ * input;
  lastFrame_[0] = b_[0] * inputs_[0] + b_[1] * inputs_[1] - a_[1] * outputs_[1];
  inputs_[1] = inputs_[0];
  outputs_[1] = lastFrame_[0];

  return lastFrame_[0];
}

void PoleZero::clear() {
  inputs_ = 0.0;
  outputs_ = 0.0;
  lastFrame_ = 0.0;
}

}  // namespace sidebands
