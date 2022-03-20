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
          size, VSTGUI::CRowColumnView::kColumnStyle,
          VSTGUI::CRowColumnView::LayoutStyle::kLeftTopEqualy, 2),
      ModulatorEditorView(edit_controller, target) {
  setBackgroundColor(VSTGUI::kTransparentCColor);

  int selected_generator = edit_controller->SelectedGenerator();

  VSTGUI::CRect column_size{0, 0, 40, getHeight()};
  sliders_ = {
      new ParameterEditorView(column_size,
                              edit_controller->FindRangedParameter(
                                  selected_generator, TAG_ENV_HT, target),
                              this, ParameterEditorStyle::VERTICAL_SHORT, "HT"),
      new ParameterEditorView(column_size,
                              edit_controller->FindRangedParameter(
                                  selected_generator, TAG_ENV_AR, target),
                              this, ParameterEditorStyle::VERTICAL_SHORT, "AR"),
      new ParameterEditorView(column_size,
                              edit_controller->FindRangedParameter(
                                  selected_generator, TAG_ENV_AL, target),
                              this, ParameterEditorStyle::VERTICAL_SHORT, "AL"),
      new ParameterEditorView(column_size,
                              edit_controller->FindRangedParameter(
                                  selected_generator, TAG_ENV_DR1, target),
                              this, ParameterEditorStyle::VERTICAL_SHORT,
                              "DR1"),
      new ParameterEditorView(column_size,
                              edit_controller->FindRangedParameter(
                                  selected_generator, TAG_ENV_DL1, target),
                              this, ParameterEditorStyle::VERTICAL_SHORT,
                              "DL1"),
      new ParameterEditorView(column_size,
                              edit_controller->FindRangedParameter(
                                  selected_generator, TAG_ENV_DR2, target),
                              this, ParameterEditorStyle::VERTICAL_SHORT,
                              "DR2"),
      new ParameterEditorView(column_size,
                              edit_controller->FindRangedParameter(
                                  selected_generator, TAG_ENV_SL, target),
                              this, ParameterEditorStyle::VERTICAL_SHORT, "S"),
      new ParameterEditorView(column_size,
                              edit_controller->FindRangedParameter(
                                  selected_generator, TAG_ENV_RR1, target),
                              this, ParameterEditorStyle::VERTICAL_SHORT,
                              "RR1"),
      new ParameterEditorView(column_size,
                              edit_controller->FindRangedParameter(
                                  selected_generator, TAG_ENV_RL1, target),
                              this, ParameterEditorStyle::VERTICAL_SHORT,
                              "RL1"),
      new ParameterEditorView(column_size,
                              edit_controller->FindRangedParameter(
                                  selected_generator, TAG_ENV_RR2, target),
                              this, ParameterEditorStyle::VERTICAL_SHORT,
                              "RR2"),
      new ParameterEditorView(column_size,
                              edit_controller->FindRangedParameter(
                                  selected_generator, TAG_ENV_VS, target),
                              this, ParameterEditorStyle::VERTICAL_SHORT, "VS"),
  };

  for (auto *slider : sliders_) {
    addView(slider);
  }
  envelope_editor_ = new GraphicalEnvelopeEditorView(
      {0, 0, 200, getHeight()}, this, edit_controller, selected_generator, target);

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