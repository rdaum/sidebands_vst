#include "controller/ui/envelope_editor_view.h"

#include "controller/sidebands_controller.h"
#include "controller/ui/graphical_envelope_editor.h"
#include "controller/ui/gui_constants.h"
#include "controller/ui/parameter_editor_view.h"
#include "tags.h"

namespace sidebands {
namespace ui {

EnvelopeEditorView::EnvelopeEditorView(const VSTGUI::CRect &size,
                                       SidebandsController *edit_controller,
                                       TargetTag target)
    : VSTGUI::CRowColumnView(
          size, VSTGUI::CRowColumnView::kRowStyle,
          VSTGUI::CRowColumnView::LayoutStyle::kLeftTopEqualy, 2),
      ModulatorEditorView(edit_controller, target) {
  setBackgroundColor(VSTGUI::kTransparentCColor);

  int selected_generator = edit_controller->SelectedGenerator();

  VSTGUI::CRect column_size{0, 0, 40, getHeight()};
  sliders_ = {
      new ParameterEditorView(column_size,
                              edit_controller->FindRangedParameter(
                                  selected_generator, TAG_ENV_HT, target),
                              this, "HT"),
      new ParameterEditorView(column_size,
                              edit_controller->FindRangedParameter(
                                  selected_generator, TAG_ENV_AR, target),
                              this, "AR"),
      new ParameterEditorView(column_size,
                              edit_controller->FindRangedParameter(
                                  selected_generator, TAG_ENV_AL, target),
                              this, "AL"),
      new ParameterEditorView(column_size,
                              edit_controller->FindRangedParameter(
                                  selected_generator, TAG_ENV_DR1, target),
                              this, "DR1"),
      new ParameterEditorView(column_size,
                              edit_controller->FindRangedParameter(
                                  selected_generator, TAG_ENV_DL1, target),
                              this, "DL1"),
      new ParameterEditorView(column_size,
                              edit_controller->FindRangedParameter(
                                  selected_generator, TAG_ENV_DR2, target),
                              this, "DR2"),
      new ParameterEditorView(column_size,
                              edit_controller->FindRangedParameter(
                                  selected_generator, TAG_ENV_SL, target),
                              this, "S"),
      new ParameterEditorView(column_size,
                              edit_controller->FindRangedParameter(
                                  selected_generator, TAG_ENV_RR1, target),
                              this, "RR1"),
      new ParameterEditorView(column_size,
                              edit_controller->FindRangedParameter(
                                  selected_generator, TAG_ENV_RL1, target),
                              this, "RL1"),
      new ParameterEditorView(column_size,
                              edit_controller->FindRangedParameter(
                                  selected_generator, TAG_ENV_RR2, target),
                              this, "RR2"),
      new ParameterEditorView(column_size,
                              edit_controller->FindRangedParameter(
                                  selected_generator, TAG_ENV_VS, target),
                              this, "VS"),
  };

  envelope_editor_ = new GraphicalEnvelopeEditorView(
      {0, 0, 200, 100}, this, edit_controller,
      selected_generator, target);

  auto *slider_row = new VSTGUI::CRowColumnView(
      VSTGUI::CRect{0, 0, getWidth(), kModRowHeight}, kColumnStyle);
  slider_row->setBackgroundColor(VSTGUI::kTransparentCColor);
  addView(slider_row);

  for (auto *slider : sliders_) {
    slider_row->addView(slider);
  }
  addView(envelope_editor_);
}

void EnvelopeEditorView::valueChanged(VSTGUI::CControl *control) {
  Steinberg::Vst::ParamID tag = control->getTag();
  edit_controller()->UpdateParameterNormalized(tag,
                                               control->getValueNormalized());
  setDirty(true);
}

void EnvelopeEditorView::SwitchGenerator(int new_generator) {
  envelope_editor_->SwitchGenerator(new_generator);

  for (auto &slider : sliders_) {
    slider->UpdateControlParameters(
        edit_controller(), edit_controller()->FindRangedParameter(
                               new_generator, slider->tag(), target()));
  }
}

} // namespace ui
} // namespace sidebands