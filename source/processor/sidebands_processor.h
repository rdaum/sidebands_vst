#pragma once

#include <pluginterfaces/vst/ivstparameterchanges.h>

#include <memory>

#include "processor/patch_processor.h"
#include "public.sdk/source/vst/vstaudioeffect.h"

namespace sidebands {

class Player;

class SidebandsProcessor : public Steinberg::Vst::AudioEffect {
 public:
  SidebandsProcessor();
  ~SidebandsProcessor() override;

  // Create function
  static Steinberg::FUnknown *Instantiate(void * /*context*/) {
    return (Steinberg::Vst::IAudioProcessor *)new SidebandsProcessor;
  }

  // AudioEffect overrides

  /** Called at first after constructor */
  Steinberg::tresult PLUGIN_API
  initialize(Steinberg::FUnknown *context) override;

  /** Called at the end before destructor */
  Steinberg::tresult PLUGIN_API terminate() override;

  /** Switch the Plug-in on/off */
  Steinberg::tresult PLUGIN_API setActive(Steinberg::TBool state) override;

  /** Will be called before any process call */
  Steinberg::tresult PLUGIN_API
  setupProcessing(Steinberg::Vst::ProcessSetup &newSetup) override;

  /** Asks if a given sample size is supported see SymbolicSampleSizes. */
  Steinberg::tresult PLUGIN_API
  canProcessSampleSize(Steinberg::int32 symbolicSampleSize) override;

  /** Here we go...the process call */
  Steinberg::tresult PLUGIN_API
  process(Steinberg::Vst::ProcessData &data) override;

  /** For persistence */
  Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream *state) override;
  Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream *state) override;
  Steinberg::tresult notify(Steinberg::Vst::IMessage *message) override;

 private:
  void SendEnvelopeStageChangedEvent(int note_id, int gennum, TargetTag target,
                                     off_t stage);

  std::unique_ptr<PatchProcessor> patch_;
  std::unique_ptr<Player> player_;
};

//------------------------------------------------------------------------
}  // namespace sidebands
