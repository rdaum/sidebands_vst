#pragma once

#include <pluginterfaces/vst/ivstparameterchanges.h>
#include <pluginterfaces/vst/vsttypes.h>
#include <public.sdk/source/vst/vstparameters.h>

#include <cassert>

namespace sidebands {

using Steinberg::int32;
using Steinberg::kResultTrue;
using Steinberg::Vst::IParamValueQueue;
using Steinberg::Vst::ParamID;
using Steinberg::Vst::ParamValue;

// Fork of Steinberg::SampleAccurate::Parameter that supports parameter value
// ranges and whatever else I need.
struct SampleAccurateValue {
  SampleAccurateValue(ParamID pid, ParamValue initValue,
                      ParamValue min, ParamValue max) noexcept;

  void setValue(ParamValue v) noexcept;
  ParamValue getValue() const noexcept;

  void setParamID(ParamID pid) noexcept;
  ParamID getParamID() const noexcept;
  bool hasChanges() const noexcept;
  void beginChanges(IParamValueQueue *valueQueue) noexcept;
  ParamValue advance(int32 numSamples) noexcept;
  ParamValue flushChanges() noexcept;
  ParamValue endChanges() noexcept;
  template <typename Proc>
  void advance(int32 numSamples, Proc p) noexcept;
  template <typename Proc>
  void flushChanges(Proc p) noexcept;
  template <typename Proc>
  void endChanges(Proc p) noexcept;

  ParamValue minPlain;
  ParamValue maxPlain;

 private:
  struct ValuePoint {
    ParamValue value{0.};
    double rampPerSample{0.};
    int32 sampleOffset{-1};

    void adjust(ParamValue min, ParamValue max) {
      value = min + (value * (max - min));
    }
  };

  ValuePoint processNextValuePoint() noexcept;

  ParamID paramID;
  int32 pointCount{-1};
  int32 pointIndex{0};
  int32 sampleCounter{0};
  ParamValue currentValue{0.};
  ValuePoint valuePoint;

  IParamValueQueue *queue{nullptr};
};

//------------------------------------------------------------------------
SMTG_ALWAYS_INLINE
SampleAccurateValue::SampleAccurateValue(ParamID pid, ParamValue initValue,
                                         ParamValue min,
                                         ParamValue max) noexcept {
  setParamID(pid);
  setValue(initValue);
  minPlain = min;
  maxPlain = max;
}

//------------------------------------------------------------------------
SMTG_ALWAYS_INLINE void SampleAccurateValue::setValue(ParamValue v) noexcept {
  currentValue = v;
  pointCount = 0;
  valuePoint = {currentValue, 0., -1};
}

//------------------------------------------------------------------------
SMTG_ALWAYS_INLINE void SampleAccurateValue::setParamID(ParamID pid) noexcept {
  paramID = pid;
}

//------------------------------------------------------------------------
SMTG_ALWAYS_INLINE ParamID SampleAccurateValue::getParamID() const noexcept {
  return paramID;
}

//------------------------------------------------------------------------
SMTG_ALWAYS_INLINE ParamValue SampleAccurateValue::getValue() const noexcept {
  return currentValue;
}

//------------------------------------------------------------------------
SMTG_ALWAYS_INLINE bool SampleAccurateValue::hasChanges() const noexcept {
  return pointCount >= 0;
}

//------------------------------------------------------------------------
SMTG_ALWAYS_INLINE void SampleAccurateValue::beginChanges(
    IParamValueQueue *valueQueue) noexcept {
  assert(queue == nullptr);
  assert(valueQueue->getParameterId() == getParamID());
  queue = valueQueue;
  pointCount = queue->getPointCount();
  pointIndex = 0;
  sampleCounter = 0;
  if (pointCount) valuePoint = processNextValuePoint();
}

//------------------------------------------------------------------------
SMTG_ALWAYS_INLINE ParamValue
SampleAccurateValue::advance(int32 numSamples) noexcept {
  if (pointCount < 0) return currentValue;
  while (valuePoint.sampleOffset >= 0 && valuePoint.sampleOffset < numSamples) {
    sampleCounter += valuePoint.sampleOffset;
    numSamples -= valuePoint.sampleOffset;
    currentValue = valuePoint.value;
    valuePoint = processNextValuePoint();
  }
  currentValue += (valuePoint.rampPerSample * numSamples);
  valuePoint.sampleOffset -= numSamples;
  sampleCounter += numSamples;
  return currentValue;
}

//------------------------------------------------------------------------
SMTG_ALWAYS_INLINE ParamValue SampleAccurateValue::flushChanges() noexcept {
  while (pointCount >= 0) {
    currentValue = valuePoint.value;
    valuePoint = processNextValuePoint();
  }
  currentValue = valuePoint.value;
  return currentValue;
}

//------------------------------------------------------------------------
SMTG_ALWAYS_INLINE ParamValue SampleAccurateValue::endChanges() noexcept {
  flushChanges();
  pointCount = -1;
  queue = nullptr;
  return currentValue;
}

//------------------------------------------------------------------------
template <typename Proc>
SMTG_ALWAYS_INLINE void SampleAccurateValue::advance(int32 numSamples,
                                                     Proc p) noexcept {
  auto originalValue = currentValue;
  if (advance(numSamples) != originalValue) {
    p(currentValue);
  }
}

//------------------------------------------------------------------------
template <typename Proc>
SMTG_ALWAYS_INLINE void SampleAccurateValue::flushChanges(Proc p) noexcept {
  auto originalValue = currentValue;
  if (flushChanges() != originalValue) p(currentValue);
}

//------------------------------------------------------------------------
template <typename Proc>
SMTG_ALWAYS_INLINE void SampleAccurateValue::endChanges(Proc p) noexcept {
  auto originalValue = currentValue;
  if (endChanges() != originalValue) p(currentValue);
}

//------------------------------------------------------------------------
SMTG_ALWAYS_INLINE auto SampleAccurateValue::processNextValuePoint() noexcept
    -> ValuePoint {
  ValuePoint nv;
  if (pointCount == 0 ||
      queue->getPoint(pointIndex, nv.sampleOffset, nv.value) != kResultTrue) {
    pointCount = -1;
    return {currentValue, 0., -1};
  }
  nv.adjust(minPlain, maxPlain);
  nv.sampleOffset -= sampleCounter;
  ++pointIndex;
  --pointCount;
  if (nv.sampleOffset == 0)
    nv.rampPerSample = (nv.value - currentValue);
  else
    nv.rampPerSample =
        (nv.value - currentValue) / static_cast<double>(nv.sampleOffset);
  return nv;
}

}  // namespace sidebands