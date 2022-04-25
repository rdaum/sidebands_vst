#pragma once

#include <bitset>

#include "processor/util/processor_param_value.h"

using Steinberg::int32;
using Steinberg::Vst::IParamValueQueue;
using Steinberg::Vst::ParamID;
using Steinberg::Vst::ParamValue;

namespace sidebands {

class Parameter : public ProcessorParameterValue {
 public:
  Parameter(ParamID param_id, ParamValue min, ParamValue max, ParamValue value);

  // ProcessorParameterValue overrides
  void setValue(ParamValue v) override;
  void setValueNormalized(ParamValue v) override;
  ParamValue getValue() const override;
  ParamValue getValueNormalized() const override;
  ParamValue Min() const override;
  ParamValue Max() const override;
  ParamID getParamID() const override;

  bool hasChanges() const override { return false; };
  void beginChanges(IParamValueQueue *valueQueue) override;
  ParamValue advance(int32 numSamples) override { return value_; };
  ParamValue flushChanges() override { return value_; };
  ParamValue endChanges() override { return value_; };

 private:
  ParamID param_id_;
  ParamValue min_plain_;
  ParamValue max_plain_;
  ParamValue value_;
};

constexpr uint8_t kBitsetWidth = 255;
class BitsetParameter : public ProcessorParameterValue {
 public:
  BitsetParameter(ParamID param_id, std::bitset<kBitsetWidth> value)
      : param_id_(param_id), value_(value) {}

  template <size_t SIZE>
  std::bitset<SIZE> bitset() const {
    static_assert(SIZE <= kBitsetWidth);
    return std::bitset<SIZE>(value_.to_ulong());
  }

  // ProcessorParameterValue overrides
  void setValue(ParamValue v) override;
  void setValueNormalized(ParamValue v) override;
  ParamValue getValue() const override;
  ParamValue getValueNormalized() const override;
  ParamValue Min() const override;
  ParamValue Max() const override;
  ParamID getParamID() const override;
  bool hasChanges() const override;
  void beginChanges(IParamValueQueue *valueQueue) override;
  ParamValue advance(int32 numSamples) override;
  ParamValue flushChanges() override;
  ParamValue endChanges() override;

 private:
  ParamID param_id_;
  std::bitset<kBitsetWidth> value_;
};

}  // namespace sidebands