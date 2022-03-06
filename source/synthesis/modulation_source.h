#pragma once

#include <pluginterfaces/vst/vsttypes.h>

#include "synthesis/patch.h"

namespace sidebands {

using Steinberg::Vst::ParamValue;
using Steinberg::Vst::SampleRate;

class IModulationSource {
 public:
  virtual void On(SampleRate sample_rate,
                  const GeneratorPatch::ModulationParameters &parameters) = 0;
  virtual void Release(
      SampleRate sample_rate,
      const GeneratorPatch::ModulationParameters &parameters) = 0;
  virtual void Reset() = 0;
  virtual ParamValue NextSample(
      SampleRate sample_rate, ParamValue velocity,
      const GeneratorPatch::ModulationParameters &parameters) = 0;
  virtual bool Playing() const = 0;
  virtual GeneratorPatch::ModType mod_type() const = 0;
};

}  // namespace sidebands