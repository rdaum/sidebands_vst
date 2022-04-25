#pragma once

#include <complex>
#include <cstddef>
#include <cstdint>
#include <valarray>

#include "dsp/dc_block.h"
#include "dsp/integrator.h"
#include "dsp/oscbuffer.h"
#include "processor/patch_processor.h"

namespace sidebands {

using Steinberg::Vst::ParamValue;

struct OscParams {
  explicit OscParams(size_t buffer_size);

  OscParam note_freq;
  OscParam C;
  OscParam M;
  OscParam R;
  OscParam S;
  OscParam K;
};

class IOscillator {
 public:
  virtual ~IOscillator() = default;
  virtual void Perform(Steinberg::Vst::SampleRate sample_rate,
                       OscBuffer &buffer, OscParams &params) = 0;
  virtual GeneratorPatch::OscType osc_type() const = 0;
  virtual void Reset() = 0;
};

std::unique_ptr<IOscillator> MakeOscillator(GeneratorPatch::OscType type);

class ModFMOscillator : public IOscillator {
 public:
  ~ModFMOscillator() override = default;
  void Perform(Steinberg::Vst::SampleRate sample_rate, OscBuffer &buffer,
               OscParams &params) override;
  void Reset() override { phase_ = 0.0f; }
  GeneratorPatch::OscType osc_type() const override {
    return GeneratorPatch::OscType::MOD_FM;
  };

 private:
  double phase_ = 0.0f;
};

// A virtual "analog" oscillator based on the same ModFM algorithm.
// Mod ratio of "2" == square.  "1" == saw.
class AnalogOscillator : public IOscillator {
 public:
  AnalogOscillator();
  ~AnalogOscillator() override = default;
  void Perform(Steinberg::Vst::SampleRate sample_rate, OscBuffer &buffer,
               OscParams &params) override;
  void Reset() override { phase_ = 0.0f; }
  GeneratorPatch::OscType osc_type() const override {
    return GeneratorPatch::OscType::ANALOG;
  };

 private:
  DCBlock2 dc_;
  Integrator int_;
  double phase_ = 0.0f;
};

}  // namespace sidebands