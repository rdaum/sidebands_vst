#include "sidebands_processor.h"

#include <base/source/fstreamer.h>
#include <chrono>
#include <glog/logging.h>
#include <pluginterfaces/vst/ivstevents.h>
#include <pluginterfaces/vst/ivstparameterchanges.h>
#include <public.sdk/source/vst/utility/processdataslicer.h>
#include <set>

#include "globals.h"
#include "sidebands_cids.h"
#include "synthesis/patch.h"
#include "synthesis/player.h"
#include "tags.h"

using namespace Steinberg;

namespace sidebands {

SidebandsProcessor::SidebandsProcessor() {
  setControllerClass(kSidebandsControllerUID);
}

SidebandsProcessor::~SidebandsProcessor() = default;

tresult PLUGIN_API SidebandsProcessor::initialize(FUnknown *context) {
  tresult result = AudioEffect::initialize(context);
  if (result != kResultOk) {
    return result;
  }

  LOG(INFO) << "Configuring buses...";
  addAudioOutput(STR16("Stereo Out"), Steinberg::Vst::SpeakerArr::kStereo);
  addEventInput(STR16("Event In"), 1);

  return kResultOk;
}

tresult PLUGIN_API SidebandsProcessor::terminate() {
  LOG(INFO) << "Terminating...";

  return AudioEffect::terminate();
}

tresult PLUGIN_API SidebandsProcessor::setActive(TBool state) {
  return AudioEffect::setActive(state);
}

tresult PLUGIN_API SidebandsProcessor::process(Vst::ProcessData &data) {
  Vst::IParameterChanges *p_changes = data.inputParameterChanges;

  // Parameter changes here.
  if (p_changes) {
    int num_changes = p_changes->getParameterCount();
    while (num_changes--) {
      auto *pq = data.inputParameterChanges->getParameterData(num_changes);
      if (pq) {
        auto param_id = pq->getParameterId();
        if (!kPatch->ValidParam(param_id)) {
          LOG(ERROR) << "Invalid parameter change: " << param_id
                     << ", ignoring";
          continue;
        }
        kPatch->BeginParameterChange(param_id, pq);
      }
    }
  }

  // Process inbound note/controller events.
  auto *input_events = data.inputEvents;
  if (input_events) {
    int num_events = input_events->getEventCount();
    for (int i = 0; i < num_events; i++) {
      Vst::Event event;
      input_events->getEvent(i, event);
      switch (event.type) {
      case Vst::Event::kNoteOnEvent:
        player_->NoteOn(std::chrono::high_resolution_clock::now(),
                        event.noteOn.noteId, event.noteOn.velocity,
                        event.noteOn.pitch);
        break;
      case Vst::Event::kNoteOffEvent:
        player_->NoteOff(event.noteOff.noteId, event.noteOff.pitch);
        break;
      case Vst::Event::kLegacyMIDICCOutEvent:
        VLOG(1) << "Legacy CC control# " << std::hex
                << (int)event.midiCCOut.controlNumber
                << " value: " << (int)event.midiCCOut.value;
        break;
      default:
        LOG(INFO) << "Other VST event: " << event.type;
        break;
      }
    }
  }

  // Process data in kSampleAccurateChunkSizeSamples so parameters changed
  // mid-buffer have a chance to be reflected on a chunk by chunk basis.
  Steinberg::Vst::ProcessDataSlicer slicer(kSampleAccurateChunkSizeSamples);
  auto processing_fn = [this](Steinberg::Vst::ProcessData &data) {
    auto *outputs = data.outputs;

    // Advance parameters to the state they'd be in this chunk.
    kPatch->AdvanceParameterChanges(kSampleAccurateChunkSizeSamples);

    // For now just the same thing on all channels. We'll eventually develop
    // stereo functionality.
    std::vector<Vst::Sample32> output_buffer(data.numSamples, 0.0);
    player_->Perform(nullptr, output_buffer.data(), data.numSamples);
    for (auto channel = 0; channel < outputs[0].numChannels; ++channel) {
      for (auto sample_index = 0; sample_index < data.numSamples;
           ++sample_index) {
        outputs[0].channelBuffers32[channel][sample_index] =
            output_buffer[sample_index];
      }
    }
  };
  slicer.process<Steinberg::Vst::kSample32>(data, processing_fn);

  kPatch->EndParameterChanges();

  return kResultOk;
}

tresult PLUGIN_API
SidebandsProcessor::setupProcessing(Vst::ProcessSetup &newSetup) {
  LOG(INFO) << "Sidebands setupProcessing sampleRate: " << newSetup.sampleRate
            << " maxSamplesPerBlock: " << newSetup.maxSamplesPerBlock
            << " patch instance: " << kPatch.get();

  player_ = std::make_unique<Player>(kPatch.get(), newSetup.sampleRate);

  LOG(INFO) << "Instantiated player...";

  //--- called before any processing ----
  return AudioEffect::setupProcessing(newSetup);
}

tresult PLUGIN_API
SidebandsProcessor::canProcessSampleSize(int32 symbolicSampleSize) {
  // by default kSample32 is supported
  if (symbolicSampleSize == Vst::kSample32)
    return kResultTrue;

  // disable the following comment if your processing support kSample64
  /* if (symbolicSampleSize == Vst::kSample64)
          return kResultTrue; */

  return kResultFalse;
}

tresult PLUGIN_API SidebandsProcessor::setState(IBStream *state) {
  // called when we load a preset, the model has to be reloaded
  IBStreamer streamer(state, kLittleEndian);

  LOG(INFO) << "Set state: " << state;

  return kResultOk;
}

tresult PLUGIN_API SidebandsProcessor::getState(IBStream *state) {
  // here we need to save the model
  IBStreamer streamer(state, kLittleEndian);

  return kResultOk;
}

} // namespace sidebands
