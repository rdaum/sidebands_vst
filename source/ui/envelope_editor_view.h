#pragma once

#include <public.sdk/source/vst/vsteditcontroller.h>
#include <vstgui/vstgui.h>

#include "constants.h"
#include "tags.h"
#include "ui/modulator_editor_view.h"

namespace sidebands {
namespace ui {

class ParameterEditorView;
class GraphicalEnvelopeEditorView;
class EnvelopeEditorView : public VSTGUI::CRowColumnView,
                           public ModulatorEditorView {
public:
  EnvelopeEditorView(const VSTGUI::CRect &size,
                     Steinberg::Vst::EditController *edit_controller,
                     TargetTag target);
  ~EnvelopeEditorView() override = default;

  void SwitchGenerator(int new_generator) override;

private:
  ParameterEditorView *a_slider_;
  ParameterEditorView *d_slider_;
  ParameterEditorView *s_slider_;
  ParameterEditorView *r_slider_;
  ParameterEditorView *vs_slider_;
  GraphicalEnvelopeEditorView *envelope_editor_;
};

}  // namespace ui
}  // namespace sidebands