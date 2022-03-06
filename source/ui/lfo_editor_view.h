#pragma once

#include <public.sdk/source/vst/vsteditcontroller.h>
#include <vstgui/vstgui.h>

#include "constants.h"
#include "tags.h"
#include "ui/modulator_editor_view.h"

namespace sidebands {
namespace ui {

class ParameterEditorView;
class LFOEditorView : public VSTGUI::CRowColumnView,
                      public ModulatorEditorView {
 public:
  LFOEditorView(const VSTGUI::CRect &size, SidebandsController *edit_controller,
                TargetTag target);
  ~LFOEditorView() override = default;

  void SwitchGenerator(int new_generator) override;

  // IDependent overloads
  void valueChanged(VSTGUI::CControl *control) override;

 private:
  ParameterEditorView *frequency_slider_;
  ParameterEditorView *amplitude_slider_;
  ParameterEditorView *vel_sens_slider_;
};

}  // namespace ui
}  // namespace sidebands