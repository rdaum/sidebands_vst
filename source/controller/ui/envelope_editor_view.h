#pragma once

#include <public.sdk/source/vst/vsteditcontroller.h>
#include <vstgui/vstgui.h>

#include "constants.h"
#include "globals.h"
#include "controller/ui/modulator_editor_view.h"
#include "tags.h"

namespace sidebands {
namespace ui {

class ParameterEditorView;
class GraphicalEnvelopeEditorView;
class EnvelopeEditorView : public VSTGUI::CRowColumnView,
                           public ModulatorEditorView {
public:
  EnvelopeEditorView(const VSTGUI::CRect &size,
                     SidebandsController *edit_controller, TargetTag target);
  ~EnvelopeEditorView() override = default;

  void SwitchGenerator(int new_generator) override;
  void HighlightEnvelopeStage(off_t stage);

  // IDependent overloads
  void valueChanged(VSTGUI::CControl *control) override;

private:
  std::vector<ParameterEditorView*> value_editors_;
  GraphicalEnvelopeEditorView *envelope_editor_;
};

} // namespace ui
} // namespace sidebands