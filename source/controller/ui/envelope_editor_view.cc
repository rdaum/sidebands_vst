#include "controller/ui/envelope_editor_view.h"

#include "controller/sidebands_controller.h"
#include "controller/ui/graphical_envelope_editor.h"
#include "controller/ui/gui_constants.h"
#include "controller/ui/parameter_editor_view.h"
#include "tags.h"

namespace sidebands {
namespace ui {

constexpr double kValueEditorWidth = 40;
constexpr double kValueEditorHeight = 80;

VSTGUI::CRowColumnView *
MakeColumn(const VSTGUI::CRect &size,
           const std::vector<ParameterEditorView *> &editors) {
  auto *column =
      new VSTGUI::CRowColumnView(size, VSTGUI::CRowColumnView::kRowStyle);
  column->setBackgroundColor(VSTGUI::kTransparentCColor);
  for (auto *editor : editors) {
    column->addView(editor);
  }
  return column;
}

EnvelopeEditorView::EnvelopeEditorView(const VSTGUI::CRect &size,
                                       SidebandsController *edit_controller,
                                       TargetTag target)
    : VSTGUI::CRowColumnView(
          size, VSTGUI::CRowColumnView::kColumnStyle,
          VSTGUI::CRowColumnView::LayoutStyle::kLeftTopEqualy, 2),
      ModulatorEditorView(edit_controller, target) {
  setBackgroundColor(VSTGUI::kTransparentCColor);

  int selected_generator = edit_controller->SelectedGenerator();

  auto editors_area = new VSTGUI::CRowColumnView(
      VSTGUI::CRect{0, 0, kValueEditorWidth * 7, size.getHeight()},
      VSTGUI::CRowColumnView::kColumnStyle);
  editors_area->setBackgroundColor(VSTGUI::kTransparentCColor);

  VSTGUI::CRect editor_size{0, 0, kValueEditorWidth, kValueEditorHeight};
  ParameterEditorView *ht_editor = new ParameterEditorView(
      editor_size,
      edit_controller->FindRangedParameter(selected_generator, TAG_ENV_HT,
                                           target),
      this, ParameterEditorStyle::KNOB, "HT");
  ParameterEditorView *ar_editor = new ParameterEditorView(
      editor_size,
      edit_controller->FindRangedParameter(selected_generator, TAG_ENV_AR,
                                           target),
      this, ParameterEditorStyle::KNOB, "AR");
  ParameterEditorView *al_editor = new ParameterEditorView(
      editor_size,
      edit_controller->FindRangedParameter(selected_generator, TAG_ENV_AL,
                                           target),
      this, ParameterEditorStyle::KNOB, "AL");
  ParameterEditorView *dr1_editor = new ParameterEditorView(
      editor_size,
      edit_controller->FindRangedParameter(selected_generator, TAG_ENV_DR1,
                                           target),
      this, ParameterEditorStyle::KNOB, "DR1");
  ParameterEditorView *dl1_editor = new ParameterEditorView(
      editor_size,
      edit_controller->FindRangedParameter(selected_generator, TAG_ENV_DL1,
                                           target),
      this, ParameterEditorStyle::KNOB, "DL1");
  ParameterEditorView *dr2_editor = new ParameterEditorView(
      editor_size,
      edit_controller->FindRangedParameter(selected_generator, TAG_ENV_DR2,
                                           target),
      this, ParameterEditorStyle::KNOB, "DR2");
  ParameterEditorView *s_editor = new ParameterEditorView(
      editor_size,
      edit_controller->FindRangedParameter(selected_generator, TAG_ENV_SL,
                                           target),
      this, ParameterEditorStyle::KNOB, "S");
  ParameterEditorView *rr1_editor = new ParameterEditorView(
      editor_size,
      edit_controller->FindRangedParameter(selected_generator, TAG_ENV_RR1,
                                           target),
      this, ParameterEditorStyle::KNOB, "RR1");
  ParameterEditorView *rl1_editor = new ParameterEditorView(
      editor_size,
      edit_controller->FindRangedParameter(selected_generator, TAG_ENV_RL1,
                                           target),
      this, ParameterEditorStyle::KNOB, "RL1");
  ParameterEditorView *rr2_editor = new ParameterEditorView(
      editor_size,
      edit_controller->FindRangedParameter(selected_generator, TAG_ENV_RR2,
                                           target),
      this, ParameterEditorStyle::KNOB, "RR2");
  ParameterEditorView *vs_editor = new ParameterEditorView(
      {0, 0, kVerticalSliderShortDimensions.width, getHeight()},
      edit_controller->FindRangedParameter(selected_generator, TAG_ENV_VS,
                                           target),
      this, ParameterEditorStyle::SLIDER_VERTICAL_SHORT, "VS");

  value_editors_ = {
      ht_editor, ar_editor,  al_editor,  dr1_editor, dl1_editor, dr2_editor,
      s_editor,  rr1_editor, rl1_editor, rr2_editor, vs_editor,
  };

  VSTGUI::CRect column_size{0, 0, kValueEditorWidth, getHeight()};

  editors_area->addView(MakeColumn(column_size, {ht_editor}));
  editors_area->addView(MakeColumn(column_size, {ar_editor, al_editor}));
  editors_area->addView(MakeColumn(column_size, {dr1_editor, dl1_editor}));
  editors_area->addView(MakeColumn(column_size, {dr2_editor, s_editor}));
  editors_area->addView(MakeColumn(column_size, {rr1_editor, rl1_editor}));
  editors_area->addView(MakeColumn(column_size, {rr2_editor}));
  editors_area->addView(vs_editor);
  addView(editors_area);

  envelope_editor_ = new GraphicalEnvelopeEditorView(
      {0, 0, 200, getHeight()}, this, edit_controller, selected_generator,
      target);

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

  for (auto &slider : value_editors_) {
    slider->UpdateControlParameters(
        edit_controller(), edit_controller()->FindRangedParameter(
                               new_generator, slider->tag(), target()));
  }
}

void EnvelopeEditorView::HighlightEnvelopeStage(off_t stage) {
  envelope_editor_->HighlightEnvelopeStage(stage);
}

} // namespace ui
} // namespace sidebands