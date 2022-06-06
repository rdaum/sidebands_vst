#define NOMINMAX
#include "controller/sidebands_controller.h"

#include <absl/strings/str_format.h>
#include <glog/logging.h>
#include <pluginterfaces/base/ustring.h>

#include "controller/patch_controller.h"
#include "dsp/fft.h"
#include "globals.h"
#include "sidebands_cids.h"
#include "tags.h"
#include "vstwebview/webview_controller_bindings.h"
#include "vstwebview/webview_pluginview.h"

using namespace Steinberg;

namespace sidebands {

void SidebandsControllerBindings::Bind(vstwebview::Webview *webview) {
  message_listener_ =
      std::make_unique<vstwebview::WebviewMessageListener>(webview);
  message_listener_->Subscribe(
      "receiveMessage", sidebands::kEnvelopeStageMessageID,
      {
          {
              kEnvelopeStageAttr,
              vstwebview::WebviewMessageListener::MessageAttribute::Type::INT,
          },
          {
              kGennumAttr,
              vstwebview::WebviewMessageListener::MessageAttribute::Type::INT,
          },
          {
              kTargetAttr,
              vstwebview::WebviewMessageListener::MessageAttribute::Type::INT,
          },
          {
              kEnvelopeStageAttr,
              vstwebview::WebviewMessageListener::MessageAttribute::Type::INT,
          },
      });
  const std::vector<vstwebview::WebviewMessageListener::MessageAttribute>
      buffer_attrs{
          {
              kSampleRateAttr,
              vstwebview::WebviewMessageListener::MessageAttribute::Type::INT,
          },
          {
              kBufferSizeAttr,
              vstwebview::WebviewMessageListener::MessageAttribute::Type::INT,
          },
          {
              kFreqAttr,
              vstwebview::WebviewMessageListener::MessageAttribute::Type::INT,
          },
          {
              kGennumAttr,
              vstwebview::WebviewMessageListener::MessageAttribute::Type::INT,
          },
          {
              kBufferDataAttr,
              vstwebview::WebviewMessageListener::MessageAttribute::Type::
                  BINARY,
          },
      };
  message_listener_->Subscribe("receiveMessage",
                               sidebands::kResponseAnalysisBufferMessageID,
                               buffer_attrs);

  message_listener_->Subscribe("receiveMessage",
                               sidebands::kResponseSpectrumBufferMessageID,
                               buffer_attrs /**/
  );
}

// static
Steinberg::FUnknown *SidebandsController::Instantiate(void *) {
  google::InitGoogleLogging("sidebands");
  FLAGS_stderrthreshold = 0;

  return (Steinberg::Vst::IEditController *)new SidebandsController;
}

tresult PLUGIN_API SidebandsController::initialize(FUnknown *context) {
  tresult result = EditControllerEx1::initialize(context);
  if (result != kResultOk) {
    return result;
  }

  LOG(INFO) << "Creating units...";
  for (int g = 0; g < kNumGenerators; g++) {
    Steinberg::Vst::UnitInfo unitInfo;
    unitInfo.id = MakeUnitID(UNIT_GENERATOR, g);
    unitInfo.parentUnitId = Steinberg::Vst::kRootUnitId;
    std::string full_name = absl::StrFormat("Generator Unit %d", g);
    Steinberg::UString(unitInfo.name, USTRINGSIZE(unitInfo.name))
        .assign(USTRING(full_name.c_str()));
    unitInfo.programListId = Steinberg::Vst::kNoProgramListId;
    Steinberg::Vst::Unit *unit = new Steinberg::Vst::Unit(unitInfo);
    addUnit(unit);
  }

  LOG(INFO) << "Declaring units and parameters";
  patch_controller_ = std::make_unique<PatchController>();
  patch_controller_->AppendParameters(&parameters);

  LOG(INFO) << "Initialization complete";

  return result;
}

tresult PLUGIN_API SidebandsController::terminate() {
  LOG(INFO) << "Terminating controller";

  //---do not forget to call parent ------
  return EditControllerEx1::terminate();
}

tresult PLUGIN_API SidebandsController::setComponentState(IBStream *state) {
  // Here you get the state of the component (Processor part)
  if (!state) return kResultFalse;

  if (patch_controller_->LoadPatch(state, this) != kResultOk)
    return kResultFalse;

  selectUnit(0);

  return kResultOk;
}

tresult PLUGIN_API SidebandsController::setState(IBStream *state) {
  // I'm not clear on what this method is for, appear to be called on load, not
  // save, and after setComponentState has already been called, and with an
  // empty stream.
  return kResultOk;
}

tresult PLUGIN_API SidebandsController::getState(IBStream *state) {
  // Here you are asked to deliver the state of the controller (if needed)
  // Note: the real state of your plug-in is saved in the processor

  return kResultTrue;
}

IPlugView *PLUGIN_API SidebandsController::createView(FIDString name) {
  webview_controller_bindings_ =
      std::make_unique<vstwebview::WebviewControllerBindings>(this);
  sidebands_controller_bindings_ =
      std::make_unique<SidebandsControllerBindings>();

  webview_pluginview_ =
      new vstwebview::WebviewPluginView(this,
                                        {webview_controller_bindings_.get(),
                                         sidebands_controller_bindings_.get()},
                                        &view_rect_);
  return webview_pluginview_;
}

void SidebandsController::UpdateParameterNormalized(
    Steinberg::Vst::ParamID param_id, Steinberg::Vst::ParamValue value) {
  setParamNormalized(param_id, value);
  beginEdit(param_id);
  performEdit(param_id, value);
  endEdit(param_id);
}

Steinberg::Vst::RangeParameter *SidebandsController::FindRangedParameter(
    uint16_t generator, const ParamTag &param, const TargetTag &sp) {
  Steinberg::Vst::ParamID tag = TagFor(generator, param, sp);
  auto *param_obj = getParameterObject(tag);
  if (!param_obj) return nullptr;
  Steinberg::Vst::RangeParameter *ranged_parameter;
  auto result = param_obj->queryInterface(Steinberg::Vst::RangeParameter::iid,
                                          (void **)&ranged_parameter);
  CHECK_EQ(result, Steinberg::kResultOk);
  return ranged_parameter;
}

Steinberg::Vst::ParamValue SidebandsController::GetParamValue(
    Steinberg::Vst::ParamID param_id) {
  auto *param_obj = getParameterObject(param_id);
  if (!param_obj) return 0;
  Steinberg::Vst::RangeParameter *ranged_parameter;
  auto result = param_obj->queryInterface(Steinberg::Vst::RangeParameter::iid,
                                          (void **)&ranged_parameter);
  CHECK_EQ(result, Steinberg::kResultOk);
  return ranged_parameter->toPlain(ranged_parameter->getNormalized());
}

Steinberg::tresult SidebandsController::beginEditFromHost(
    Steinberg::Vst::ParamID paramID) {
  return beginEdit(paramID);
}

Steinberg::tresult SidebandsController::endEditFromHost(
    Steinberg::Vst::ParamID paramID) {
  return endEdit(paramID);
}

Steinberg::tresult SidebandsController::ProduceFFTResponseMessageFor(
    Steinberg::Vst::IMessage *message) {
  auto analysis_attrs = message->getAttributes();
  int64 sample_rate, buffer_size, analysis_note, gennum;
  if (analysis_attrs->getInt(kSampleRateAttr, sample_rate) !=
      Steinberg::kResultOk)
    return Steinberg::kResultFalse;
  if (analysis_attrs->getInt(kBufferSizeAttr, buffer_size) !=
      Steinberg::kResultOk)
    return Steinberg::kResultFalse;
  if (analysis_attrs->getInt(kFreqAttr, analysis_note) != Steinberg::kResultOk)
    return Steinberg::kResultFalse;
  if (analysis_attrs->getInt(kGennumAttr, gennum) != Steinberg::kResultOk)
    return Steinberg::kResultFalse;

  const double *buffer_data;
  uint32 buffer_data_size;
  if (analysis_attrs->getBinary(kBufferDataAttr, (const void *&)buffer_data,
                                buffer_data_size) != Steinberg::kResultOk)
    return Steinberg::kResultFalse;
  auto fft_buffer =
      ScalarToComplex(buffer_data, buffer_data_size / sizeof(double));
  HanningWindow(fft_buffer);
  FFT(fft_buffer);
  size_t sbuffer_size = fft_buffer.size() / 2;
  std::valarray<double> sbuffer(sbuffer_size);
  for (int i = 0; i < sbuffer_size; i++) {
    sbuffer[i] = std::abs(fft_buffer[i]);
  }
  sbuffer = sbuffer / (sbuffer.max() / 2);
  sbuffer -= 1;
  analysis_attrs->setBinary(kBufferDataAttr, &sbuffer[0],
                            sbuffer_size * sizeof(double));
  analysis_attrs->setInt(kBufferSizeAttr, sbuffer_size);
  analysis_attrs->setInt(kGennumAttr, gennum);

  return Steinberg::kResultTrue;
}

Steinberg::tresult SidebandsController::notify(
    Steinberg::Vst::IMessage *message) {
  auto *ml = sidebands_controller_bindings_->message_listener();
  if (!ml) return ComponentBase::notify(message);

  tresult res;

  // Intercept buffer responses for spectrum and transform it to provide an FFT.
  if (FIDStringsEqual(message->getMessageID(),
                      kResponseSpectrumBufferMessageID)) {
    res = ProduceFFTResponseMessageFor(message);
    if (res != Steinberg::kResultOk) return res;
  }

  res = ml->Notify(message);
  if (res != Steinberg::kResultOk) return ComponentBase::notify(message);
  return res;
}

}  // namespace sidebands
