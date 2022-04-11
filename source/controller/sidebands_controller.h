#pragma once

#include <public.sdk/source/vst/vsteditcontroller.h>

#include "tags.h"
#include "globals.h"
#include "constants.h"
#include "controller/webview_pluginview.h"

namespace sidebands {

class WebviewControllerBindings;

class PatchController;
class SidebandsController : public Steinberg::Vst::EditControllerEx1,
                            public Steinberg::Vst::IEditControllerHostEditing {
public:
  static Steinberg::FUnknown *Instantiate(void * /*context*/);

  SidebandsController() = default;
  ~SidebandsController() override = default;

  // Update the parameter in such a way that the changes are propagated to the
  // processor.
  void UpdateParameterNormalized(Steinberg::Vst::ParamID param_id,
                                 Steinberg::Vst::ParamValue value);

  // Get a parameter value in its native range (not normalized).
  Steinberg::Vst::ParamValue GetParamValue(Steinberg::Vst::ParamID param_id);

  Steinberg::Vst::RangeParameter *FindRangedParameter(uint16_t generator,
                                                      const ParamTag &param,
                                                      const TargetTag &sp);
  // IPluginBase overrides
  Steinberg::tresult PLUGIN_API
  initialize(Steinberg::FUnknown *context) override;
  Steinberg::tresult PLUGIN_API terminate() override;

  // EditController overrides
  Steinberg::tresult PLUGIN_API
  setComponentState(Steinberg::IBStream *state) override;
  Steinberg::IPlugView *PLUGIN_API
  createView(Steinberg::FIDString name) override;
  Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream *state) override;
  Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream *state) override;

  // IEditControllerHostEditing overrides
  Steinberg::tresult PLUGIN_API beginEditFromHost (Steinberg::Vst::ParamID paramID) override;
  Steinberg::tresult PLUGIN_API endEditFromHost (Steinberg::Vst::ParamID paramID) override;

  // IComponentBase overrides
  Steinberg::tresult PLUGIN_API notify (Steinberg::Vst::IMessage* message) override;

  //---Interface---------
  DEFINE_INTERFACES
  DEF_INTERFACE(IUnitInfo)
  DEF_INTERFACE(IEditControllerHostEditing)
  // Here you can add more supported VST3 interfaces
  // DEF_INTERFACE (Vst::IXXX)
  END_DEFINE_INTERFACES(EditController)
  DELEGATE_REFCOUNT(EditController)

private:
  Steinberg::tresult ProduceFFTResponseMessageFor(Steinberg::Vst::IMessage *message);

  std::unique_ptr<PatchController> patch_controller_;
  Steinberg::ViewRect view_rect_{0, 0, 1024, 1200};
  std::unique_ptr<WebviewControllerBindings> webview_controller_bindings_;
  sidebands::ui::WebviewPluginView *webview_pluginview_;

};

//------------------------------------------------------------------------
} // namespace sidebands
