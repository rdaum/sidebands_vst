#pragma once

#include <pluginterfaces/vst/vsttypes.h>

#include "dsp.h"
#include "processor/patch_processor.h"

namespace sidebands {

using Steinberg::Vst::ParamValue;
using Steinberg::Vst::SampleRate;

class IModulationSource {
public:
  virtual void On(SampleRate sample_rate,
                  const GeneratorPatch::ModTarget *parameters) = 0;
  virtual void Release(SampleRate sample_rate,
                       const GeneratorPatch::ModTarget *parameters) = 0;
  virtual void Reset() = 0;
  virtual void Amplitudes(SampleRate sample_rate, OscBuffer &buffer,
                          ParamValue velocity,
                          const GeneratorPatch::ModTarget *parameters) = 0;
  virtual bool Playing() const = 0;
  virtual ModType mod_type() const = 0;
};

} // namespace sidebands