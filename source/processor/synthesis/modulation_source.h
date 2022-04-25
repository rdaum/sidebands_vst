#pragma once

#include <pluginterfaces/vst/vsttypes.h>

#include "dsp/oscbuffer.h"
#include "globals.h"
#include "processor/patch_processor.h"

namespace sidebands {

using Steinberg::Vst::ParamValue;
using Steinberg::Vst::SampleRate;

class IModulationSource {
 public:
  virtual void On(SampleRate sample_rate,
                  const GeneratorPatch::ModParams *parameters) = 0;
  virtual void Release(SampleRate sample_rate,
                       const GeneratorPatch::ModParams *parameters) = 0;
  virtual void Reset() = 0;
  virtual void Amplitudes(SampleRate sample_rate, OscBuffer &buffer,
                          ParamValue velocity,
                          const GeneratorPatch::ModParams *parameters) = 0;
  virtual bool Playing() const = 0;
  virtual Modulation::Type mod_type() const = 0;
};

}  // namespace sidebands