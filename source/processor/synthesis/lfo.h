#pragma once

#include "processor/synthesis/modulation_source.h"

namespace sidebands {

class LFO : public IModulationSource {
public:
  // IModulationSource overrides
  void On(SampleRate sample_rate,
          const GeneratorPatch::ModTarget *parameters) override;
  void Release(SampleRate sample_rate,
               const GeneratorPatch::ModTarget *parameters) override;
  void Reset() override;
  ParamValue NextSample(SampleRate sample_rate, ParamValue velocity,
                        const GeneratorPatch::ModTarget *parameters) override;
  bool Playing() const override;
  ModType mod_type() const override;

private:
  double phase_ = 0.0;
  bool playing_ = false;
};

} // namespace sidebands
