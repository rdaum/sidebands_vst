#include "controller/ui/envelope_editor_view.h"

#include "tags.h"
#include "controller/sidebands_controller.h"
#include "controller/ui/graphical_envelope_editor.h"
#include "controller/ui/parameter_editor_view.h"

namespace sidebands {
namespace ui {

EnvelopeEditorView::EnvelopeEditorView(const VSTGUI::CRect &size,
                                       SidebandsController *edit_controller,
                                       TargetTag target)
    : VSTGUI::CRowColumnView(
          size, VSTGUI::CRowColumnView::kColumnStyle,
          VSTGUI::CRowColumnView::LayoutStyle::kLeftTopEqualy, 2),
      ModulatorEditorView(edit_controller, target) {
  setBackgroundColor(kBgGrey);
  int selected_generator = edit_controller->SelectedGenerator();

  VSTGUI::CRect column_size{0, 0, 40, getHeight()};
  a_slider_ =
      new ParameterEditorView(column_size,
                              edit_controller->FindRangedParameter(
                                  selected_generator, TAG_ENV_A, target),
                              this, "A");
  d_slider_ =
      new ParameterEditorView(column_size,
                              edit_controller->FindRangedParameter(
                                  selected_generator, TAG_ENV_D, target),
                              this, "D");
  s_slider_ =
      new ParameterEditorView(column_size,
                              edit_controller->FindRangedParameter(
                                  selected_generator, TAG_ENV_S, target),
                              this, "S");
  r_slider_ =
      new ParameterEditorView(column_size,
                              edit_controller->FindRangedParameter(
                                  selected_generator, TAG_ENV_R, target),
                              this, "R");
  vs_slider_ =
      new ParameterEditorView(column_size,
                              edit_controller->FindRangedParameter(
                                  selected_generator, TAG_ENV_VS, target),
                              this, "VS");

  envelope_editor_ = new GraphicalEnvelopeEditorView(
      {0, 0, 400, getHeight()}, this, edit_controller, selected_generator,
      target);

  addView(a_slider_);
  addView(d_slider_);
  addView(s_slider_);
  addView(r_slider_);
  addView(vs_slider_);
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

  a_slider_->UpdateControlParameters(edit_controller(),
                                     edit_controller()->FindRangedParameter(
                                         new_generator, TAG_ENV_A, target()));
  d_slider_->UpdateControlParameters(edit_controller(),
                                     edit_controller()->FindRangedParameter(
                                         new_generator, TAG_ENV_D, target()));
  s_slider_->UpdateControlParameters(edit_controller(),
                                     edit_controller()->FindRangedParameter(
                                         new_generator, TAG_ENV_S, target()));
  r_slider_->UpdateControlParameters(edit_controller(),
                                     edit_controller()->FindRangedParameter(
                                         new_generator, TAG_ENV_R, target()));
  vs_slider_->UpdateControlParameters(edit_controller(),
                                      edit_controller()->FindRangedParameter(
                                          new_generator, TAG_ENV_VS, target()));
}

} // namespace ui
} // namespace sidebands