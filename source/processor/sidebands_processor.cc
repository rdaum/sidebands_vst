#include "sidebands_processor.h"

#include <base/source/fstreamer.h>
#include <glog/logging.h>
#include <pluginterfaces/vst/ivstevents.h>
#include <pluginterfaces/vst/ivstparameterchanges.h>
#include <public.sdk/source/vst/utility/processdataslicer.h>

#include <chrono>
#include <set>

#include "globals.h"
#include "processor/patch_processor.h"
#include "processor/synthesis/player.h"
#include "sidebands_cids.h"
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

  LOG(INFO) << "Creating patch storage...";
  patch_ = std::make_unique<PatchProcessor>();

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
        if (!PatchProcessor::ValidParam(param_id)) {
          LOG(ERROR) << "Invalid parameter change: " << param_id
                     << ", ignoring";
          continue;
        }
        patch_->BeginParameterChange(param_id, pq);
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
          LOG(INFO) << "Other VST event type: " << event.type;
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
    patch_->AdvanceParameterChanges(kSampleAccurateChunkSizeSamples);

    // For now just the same thing on all channels. We'll eventually develop
    // stereo functionality.
    if (data.symbolicSampleSize ==
        Steinberg::Vst::SymbolicSampleSizes::kSample32) {
      std::vector<Vst::Sample32> output_buffer(data.numSamples, 0.0);
      player_->Perform32(nullptr, output_buffer.data(), data.numSamples);
      for (auto channel = 0; channel < outputs[0].numChannels; ++channel) {
        for (auto sample_index = 0; sample_index < data.numSamples;
             ++sample_index) {
          outputs[0].channelBuffers32[channel][sample_index] =
              output_buffer[sample_index];
        }
      }
      return;
    }

    if (data.symbolicSampleSize ==
        Steinberg::Vst::SymbolicSampleSizes::kSample64) {
      std::vector<Vst::Sample64> output_buffer(data.numSamples, 0.0);
      player_->Perform64(nullptr, output_buffer.data(), data.numSamples);
      for (auto channel = 0; channel < outputs[0].numChannels; ++channel) {
        for (auto sample_index = 0; sample_index < data.numSamples;
             ++sample_index) {
          outputs[0].channelBuffers64[channel][sample_index] =
              output_buffer[sample_index];
        }
      }
      return;
    }
  };
  slicer.process<Steinberg::Vst::kSample32>(data, processing_fn);

  patch_->EndParameterChanges();

  return kResultOk;
}

tresult SidebandsProcessor::notify(Vst::IMessage *message) {
  if (!FIDStringsEqual(message->getMessageID(),
                       kRequestAnalysisBufferMessageID) &&
      !FIDStringsEqual(message->getMessageID(),
                       kRequestSpectrumBufferMessageID)) {
    return ComponentBase::notify(message);
  }

  auto attributes = message->getAttributes();

  int64 buffer_size;
  int64 frequency;
  int64 sample_rate;
  int64 gennum;
  attributes->getInt(kBufferSizeAttr, buffer_size);
  attributes->getInt(kFreqAttr, frequency);
  attributes->getInt(kSampleRateAttr, sample_rate);
  attributes->getInt(kGennumAttr, gennum);

  OscBuffer buffer(buffer_size);

  // Produce buffer by making a one-off generator. Unless the generator is
  // -1, in which case do a bunch and mix them together.
  if (gennum == -1) {
    buffer = 0.0;
    for (auto &generator : patch_->generators_) {
      if (!generator->on()) continue;
      OscBuffer mix_buffer(buffer_size);
      Generator analysis_generator;
      analysis_generator.Synthesize(sample_rate, *generator, mix_buffer,
                                    frequency);
      mix_buffer *= generator->a();
      buffer += mix_buffer;
    }
  } else {
    Generator analysis_generator;
    analysis_generator.Synthesize(sample_rate, *patch_->generators_[gennum],
                                  buffer, frequency);
  }

  // Send a response with the buffer data.
  if (auto env_change_message = owned(allocateMessage())) {
    if (FIDStringsEqual(message->getMessageID(),
                        kRequestAnalysisBufferMessageID))
      env_change_message->setMessageID(kResponseAnalysisBufferMessageID);
    else
      env_change_message->setMessageID(kResponseSpectrumBufferMessageID);

    auto *resp_attributes = env_change_message->getAttributes();
    resp_attributes->setInt(kSampleRateAttr, sample_rate);
    resp_attributes->setInt(kBufferSizeAttr, buffer.size());
    resp_attributes->setInt(kGennumAttr, gennum);
    resp_attributes->setInt(kFreqAttr, frequency);

    resp_attributes->setBinary(kBufferDataAttr, &buffer[0],
                               buffer.size() * sizeof(double));
    sendMessage(env_change_message);

    return Steinberg::kResultOk;
  }

  return Steinberg::kResultFalse;
}

tresult PLUGIN_API
SidebandsProcessor::setupProcessing(Vst::ProcessSetup &newSetup) {
  LOG(INFO) << "Sidebands setupProcessing sampleRate: " << newSetup.sampleRate
            << " maxSamplesPerBlock: " << newSetup.maxSamplesPerBlock
            << " patch instance: " << patch_.get();

  player_ = std::make_unique<Player>(patch_.get(), newSetup.sampleRate);
  // Connect asynchronous events to update the UI.
  player_->events.EnvelopeStageChange.connect(
      &SidebandsProcessor::SendEnvelopeStageChangedEvent, this);

  return AudioEffect::setupProcessing(newSetup);
}

tresult PLUGIN_API
SidebandsProcessor::canProcessSampleSize(int32 symbolicSampleSize) {
  if (symbolicSampleSize == Vst::kSample32) return kResultTrue;

  if (symbolicSampleSize == Vst::kSample64) return kResultTrue;

  return kResultFalse;
}

tresult PLUGIN_API SidebandsProcessor::setState(IBStream *state) {
  // Here you set the state of the component (Processor part)
  if (!state) return kResultFalse;

  return patch_->LoadPatch(state);
}

tresult PLUGIN_API SidebandsProcessor::getState(IBStream *state) {
  // Here you get the state of the component (Processor part)
  if (!state) return kResultFalse;

  return patch_->SavePatch(state);
}

void SidebandsProcessor::SendEnvelopeStageChangedEvent(int note_id, int gennum,
                                                       TargetTag target,
                                                       off_t stage) {
  if (auto env_change_message = owned(allocateMessage())) {
    env_change_message->setMessageID(kEnvelopeStageMessageID);
    auto *attributes = env_change_message->getAttributes();
    attributes->setInt(kNoteIdAttr, note_id);
    attributes->setInt(kGennumAttr, gennum);
    attributes->setInt(kTargetAttr, target);
    attributes->setInt(kEnvelopeStageAttr, stage);
    sendMessage(env_change_message);
  }
}

}  // namespace sidebands
