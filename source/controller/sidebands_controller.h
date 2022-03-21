#pragma once

#include <public.sdk/source/vst/vsteditcontroller.h>
#include <vstgui/plugin-bindings/vst3editor.h>

#include "tags.h"
#include "globals.h"

namespace sidebands {

namespace ui {
class GeneratorEditorView;
} // namespace ui

class PatchController;
class SidebandsController : public Steinberg::Vst::EditControllerEx1,
                            public Steinberg::Vst::IEditControllerHostEditing,
                            public VSTGUI::VST3EditorDelegate {
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
  void SelectGenerator(int generator_number);
  int SelectedGenerator();

  // EditControllerEx1 overrides
  VSTGUI::CView *createCustomView(VSTGUI::UTF8StringPtr name,
                                  const VSTGUI::UIAttributes &attributes,
                                  const VSTGUI::IUIDescription *description,
                                  VSTGUI::VST3Editor *editor) override;

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
  Steinberg::tresult PLUGIN_API setParamNormalized(
      Steinberg::Vst::ParamID tag, Steinberg::Vst::ParamValue value) override;
  Steinberg::tresult PLUGIN_API getParamStringByValue(
      Steinberg::Vst::ParamID tag, Steinberg::Vst::ParamValue valueNormalized,
      Steinberg::Vst::String128 string) override;
  Steinberg::tresult PLUGIN_API getParamValueByString(
      Steinberg::Vst::ParamID tag, Steinberg::Vst::TChar *string,
      Steinberg::Vst::ParamValue &valueNormalized) override;

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
  VSTGUI::CView *analysis_view_ = nullptr;
  ui::GeneratorEditorView *generator_view_ = nullptr;
  std::unique_ptr<PatchController> patch_controller_;
};

//------------------------------------------------------------------------
} // namespace sidebands
