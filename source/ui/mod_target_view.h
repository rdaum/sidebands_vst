#pragma once

#include <public.sdk/source/vst/vsteditcontroller.h>
#include <vstgui/vstgui.h>

#include "constants.h"
#include "tags.h"
#include "ui/patch_param_view.h"
#include "ui/modulator_editor_view.h"

namespace sidebands {

class EnvelopeEditorView;
class LFOEditorView;
class ModulatorTargetView : public VSTGUI::CRowColumnView,
                            ModulatorEditorView {
public:
  ModulatorTargetView(const VSTGUI::CRect &size,
                      Steinberg::Vst::EditController *edit_controller,
                      TargetTag target);
  ~ModulatorTargetView() override = default;

  void SwitchGenerator(int new_generator) override;

private:
  void valueChanged(VSTGUI::CControl *control) override;

private:
  VSTGUI::COptionMenu *mod_source_selector_;
  EnvelopeEditorView *envelope_editor_view_;
  LFOEditorView *lfo_editor_view_;
  void SwitchViewVisibility();
};

} // namespace sidebands
