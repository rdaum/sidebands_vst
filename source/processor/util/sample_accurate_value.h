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
// in ranges and whatever else I end up needing.
class SampleAccurateValue {
public:
  SampleAccurateValue(ParamID pid, ParamValue initValue, ParamValue min,
                      ParamValue max) ;

  void setValue(ParamValue v) ;
  void setValueNormalized(ParamValue v) ;
  ParamValue getValue() const ;
  ParamValue getValueNormalized() const ;
  ParamValue Min() const { return min_plain_; }
  ParamValue Max() const { return max_plain_; }
  ParamID getParamID() const ;
  bool hasChanges() const ;
  void beginChanges(IParamValueQueue *valueQueue) ;
  ParamValue advance(int32 numSamples) ;
  ParamValue flushChanges() ;
  ParamValue endChanges() ;
  template <typename Proc> void advance(int32 numSamples, Proc p) ;
  template <typename Proc> void flushChanges(Proc p) ;
  template <typename Proc> void endChanges(Proc p) ;

private:
  struct ValuePoint {
    ParamValue value = 0.;
    double rampPerSample = 0.;
    int32 sampleOffset = -1;
  };
  ValuePoint processNextValuePoint() ;

  const ParamID paramID_;
  const ParamValue min_plain_;
  const ParamValue max_plain_;

  int32 point_count_ = -1;
  int32 point_index_ = 0;
  int32 sample_counter_ = 0;
  ParamValue current_value_ = 0.0;
  ValuePoint valuePoint_;

  IParamValueQueue *queue_ = nullptr;
};

} // namespace sidebands