#pragma once

#include <pluginterfaces/vst/ivstparameterchanges.h>
#include <pluginterfaces/vst/vsttypes.h>

using Steinberg::int32;
using Steinberg::Vst::IParamValueQueue;
using Steinberg::Vst::ParamID;
using Steinberg::Vst::ParamValue;

namespace sidebands {
class ProcessorParameterValue {
 public:
  virtual void setValue(ParamValue v) = 0;
  virtual void setValueNormalized(ParamValue v) = 0;
  virtual ParamValue getValue() const = 0;
  virtual ParamValue getValueNormalized() const = 0;
  virtual ParamValue Min() const = 0;
  virtual ParamValue Max() const = 0;
  virtual ParamID getParamID() const = 0;

  virtual bool hasChanges() const = 0;
  virtual void beginChanges(IParamValueQueue *valueQueue) = 0;
  virtual ParamValue advance(int32 numSamples) = 0;
  virtual ParamValue flushChanges() = 0;
  virtual ParamValue endChanges() = 0;
};

}  // namespace sidebands