#include "processor/util/sample_accurate_value.h"

namespace sidebands {

SampleAccurateValue::SampleAccurateValue(ParamID pid, ParamValue initValue,
                                         ParamValue min, ParamValue max)
    : paramID_(pid), min_plain_(min), max_plain_(max) {
  setValue(initValue);
}

void SampleAccurateValue::setValue(ParamValue v) {
  v = (v - min_plain_) / (max_plain_ - min_plain_);
  setValueNormalized(v);
}

void SampleAccurateValue::setValueNormalized(ParamValue v) {
  current_value_ = v;
  point_count_ = 0;
  valuePoint_ = {current_value_, 0., -1};
}

ParamID SampleAccurateValue::getParamID() const { return paramID_; }

ParamValue SampleAccurateValue::getValue() const {
  return min_plain_ + (current_value_ * (max_plain_ - min_plain_));
}

ParamValue SampleAccurateValue::getValueNormalized() const {
  return current_value_;
}

bool SampleAccurateValue::hasChanges() const { return point_count_ >= 0; }

void SampleAccurateValue::beginChanges(IParamValueQueue *valueQueue) {
  assert(queue_ == nullptr);
  assert(valueQueue->getParameterId() == getParamID());
  queue_ = valueQueue;
  point_count_ = queue_->getPointCount();
  point_index_ = 0;
  sample_counter_ = 0;
  if (point_count_) valuePoint_ = processNextValuePoint();
}

ParamValue SampleAccurateValue::advance(int32 numSamples) {
  if (point_count_ < 0) return current_value_;
  while (valuePoint_.sampleOffset >= 0 &&
         valuePoint_.sampleOffset < numSamples) {
    sample_counter_ += valuePoint_.sampleOffset;
    numSamples -= valuePoint_.sampleOffset;
    current_value_ = valuePoint_.value;
    valuePoint_ = processNextValuePoint();
  }
  current_value_ += (valuePoint_.rampPerSample * numSamples);
  valuePoint_.sampleOffset -= numSamples;
  sample_counter_ += numSamples;
  return current_value_;
}

ParamValue SampleAccurateValue::flushChanges() {
  while (point_count_ >= 0) {
    current_value_ = valuePoint_.value;
    valuePoint_ = processNextValuePoint();
  }
  current_value_ = valuePoint_.value;
  return current_value_;
}

ParamValue SampleAccurateValue::endChanges() {
  flushChanges();
  point_count_ = -1;
  queue_ = nullptr;
  return current_value_;
}

template <typename Proc>
void SampleAccurateValue::advance(int32 numSamples, Proc p) {
  auto originalValue = current_value_;
  if (advance(numSamples) != originalValue) {
    p(current_value_);
  }
}

template <typename Proc>
void SampleAccurateValue::flushChanges(Proc p) {
  auto originalValue = current_value_;
  if (flushChanges() != originalValue) p(current_value_);
}

template <typename Proc>
void SampleAccurateValue::endChanges(Proc p) {
  auto originalValue = current_value_;
  if (endChanges() != originalValue) p(current_value_);
}

auto SampleAccurateValue::processNextValuePoint() -> ValuePoint {
  ValuePoint nv;
  if (point_count_ == 0 || queue_->getPoint(point_index_, nv.sampleOffset,
                                            nv.value) != kResultTrue) {
    point_count_ = -1;
    return {current_value_, 0., -1};
  }
  nv.sampleOffset -= sample_counter_;
  ++point_index_;
  --point_count_;
  if (nv.sampleOffset == 0)
    nv.rampPerSample = (nv.value - current_value_);
  else
    nv.rampPerSample =
        (nv.value - current_value_) / static_cast<double>(nv.sampleOffset);
  return nv;
}

}  // namespace sidebands