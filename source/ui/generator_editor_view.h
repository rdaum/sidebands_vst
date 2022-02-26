#pragma once

#include <public.sdk/source/vst/vsteditcontroller.h>
#include <vstgui/vstgui.h>

#include "constants.h"
#include "tags.h"
#include "ui/patch_param_view.h"

namespace sidebands {

class SidebandsController;
class ParameterEditorView;
class EnvelopeEditorView;
class ModulatorTargetView;
class GeneratorEditorView : public VSTGUI::CScrollView, Steinberg::FObject {
public:
  GeneratorEditorView(const VSTGUI::CRect &size,
                      Steinberg::Vst::EditController *edit_controller);
  ~GeneratorEditorView() override;

  // IControlListener overrides
  void valueChanged(VSTGUI::CControl *control) override;

  // IDependent overrides
  void update(FUnknown *unknown, Steinberg::int32 int_32) override;

private:
  Steinberg::Vst::EditController *edit_controller_;
  VSTGUI::CTextLabel *selected_label_;
  ParameterEditorView *c_slider_;
  ParameterEditorView *m_slider_;
  ParameterEditorView *k_slider_;

  ModulatorTargetView *a_target_view_;
  ModulatorTargetView *k_target_view_;
};

} // namespace sidebands