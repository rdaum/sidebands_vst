#include "processor/util/parameter.h"

#include <pluginterfaces/vst/ivstparameterchanges.h>

#include <optional>

namespace sidebands {

namespace {

std::optional<ParamValue> GetLastValue(
    Steinberg::Vst::IParamValueQueue *p_queue) {
  ParamValue value;
  int32_t numPoints = p_queue->getPointCount();
  Steinberg::int32 sample_offset;
  if (p_queue->getPoint(numPoints - 1, sample_offset, value) ==
      Steinberg::kResultTrue) {
    return value;
  }
  return std::nullopt;
}

}  // namespace

Parameter::Parameter(ParamID param_id, ParamValue min, ParamValue max,
                     ParamValue value)
    : param_id_(param_id), min_plain_(min), max_plain_(max), value_(value) {}

void Parameter::setValue(ParamValue v) {
  v = (v - min_plain_) / (max_plain_ - min_plain_);
  setValueNormalized(v);
}

void Parameter::setValueNormalized(ParamValue v) { value_ = v; }

ParamValue Parameter::getValue() const {
  return min_plain_ + (value_ * (max_plain_ - min_plain_));
}
ParamValue Parameter::getValueNormalized() const { return value_; }
ParamValue Parameter::Min() const { return min_plain_; }
ParamValue Parameter::Max() const { return max_plain_; }
ParamID Parameter::getParamID() const { return param_id_; }

void Parameter::beginChanges(IParamValueQueue *p_queue) {
  const auto &last_v_opt = GetLastValue(p_queue);
  if (last_v_opt) {
    setValueNormalized(last_v_opt.value());
  }
}

void BitsetParameter::setValue(ParamValue v) {
  value_ = std::bitset<kBitsetWidth>(v);
}

void BitsetParameter::setValueNormalized(ParamValue v) {
  value_ = std::bitset<kBitsetWidth>(v * double(kBitsetWidth));
}

ParamValue BitsetParameter::getValue() const { return value_.to_ulong(); }

ParamValue BitsetParameter::getValueNormalized() const {
  return value_.to_ulong() / double(kBitsetWidth);
}

ParamValue BitsetParameter::Min() const { return 0; }
ParamValue BitsetParameter::Max() const { return kBitsetWidth; }
ParamID BitsetParameter::getParamID() const { return param_id_; }

bool BitsetParameter::hasChanges() const { return false; }
void BitsetParameter::beginChanges(IParamValueQueue *p_queue) {
  const auto &last_v_opt = GetLastValue(p_queue);
  if (last_v_opt) {
    setValueNormalized(last_v_opt.value());
  }
}

ParamValue BitsetParameter::advance(int32 numSamples) {
  return getValueNormalized();
}
ParamValue BitsetParameter::flushChanges() { return getValueNormalized(); }
ParamValue BitsetParameter::endChanges() { return getValueNormalized(); }

}  // namespace sidebands