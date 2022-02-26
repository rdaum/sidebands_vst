#pragma once

#include <public.sdk/source/vst/vsteditcontroller.h>
#include <vstgui/vstgui.h>

#include "constants.h"
#include "tags.h"
#include "ui/patch_param_view.h"

namespace sidebands {

class ParameterEditorView;
class EnvelopeEditorView;
class ModulatorTargetView : public VSTGUI::CRowColumnView,
                            Steinberg::FObject,
                            VSTGUI::IControlListener {
public:
  ModulatorTargetView(const VSTGUI::CRect &size,
                      Steinberg::Vst::EditController *edit_controller,
                      TargetTag target);
  ~ModulatorTargetView() override = default;

  void SwitchGenerator(int new_generator);

  // IControlListener overrides
  void valueChanged(VSTGUI::CControl *control) override;

  // IDependent overrides
  void update(FUnknown *unknown, Steinberg::int32 int_32) override;

private:
  Steinberg::Vst::EditController *edit_controller_;
  const TargetTag target_;

  ParameterEditorView *a_slider_;
  ParameterEditorView *d_slider_;
  ParameterEditorView *s_slider_;
  ParameterEditorView *r_slider_;

  EnvelopeEditorView *envelope_editor_;
};

} // namespace sidebands
