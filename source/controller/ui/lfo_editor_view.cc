#include "controller/ui/lfo_editor_view.h"

#include "controller/sidebands_controller.h"
#include "controller/ui/parameter_editor_view.h"

namespace sidebands {
namespace ui {

LFOEditorView::LFOEditorView(const VSTGUI::CRect &size,
                             SidebandsController *edit_controller,
                             TargetTag target)
    : VSTGUI::CRowColumnView(
          size, VSTGUI::CRowColumnView::kColumnStyle,
          VSTGUI::CRowColumnView::LayoutStyle::kLeftTopEqualy, 2),
      ModulatorEditorView(edit_controller, target) {
  setBackgroundColor(kBgGrey);
  int selected_generator = edit_controller->SelectedGenerator();

  VSTGUI::CRect column_size{0, 0, 40, getHeight()};
  frequency_slider_ = new ParameterEditorView(
      column_size,
      edit_controller->FindRangedParameter(selected_generator, TAG_LFO_FREQ,
                          target),
      this, "Freq");
  amplitude_slider_ = new ParameterEditorView(
      column_size,
      edit_controller->FindRangedParameter(selected_generator, TAG_LFO_AMP,
                          target),
      this, "Amp");
  vel_sens_slider_ = new ParameterEditorView(
      column_size,
      edit_controller->FindRangedParameter(selected_generator, TAG_LFO_VS,
                          target),
      this, "VS");
  addView(frequency_slider_);
  addView(amplitude_slider_);
  addView(vel_sens_slider_);
}

void LFOEditorView::SwitchGenerator(int new_generator) {
  frequency_slider_->UpdateControlParameters(
      edit_controller(), edit_controller()->FindRangedParameter(new_generator,
                                             TAG_LFO_FREQ, target()));

  amplitude_slider_->UpdateControlParameters(
      edit_controller(), edit_controller()->FindRangedParameter(new_generator,
                                             TAG_LFO_AMP, target()));

  vel_sens_slider_->UpdateControlParameters(
      edit_controller(), edit_controller()->FindRangedParameter(new_generator,
                                             TAG_LFO_VS, target()));
  setDirty(true);
}

void LFOEditorView::valueChanged(VSTGUI::CControl *control) {
  Steinberg::Vst::ParamID tag = control->getTag();
  edit_controller()->UpdateParameterNormalized(tag,
                                               control->getValueNormalized());

  setDirty(true);
}
}  // namespace ui
}  // namespace sidebands