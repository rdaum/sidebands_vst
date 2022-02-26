#include "ui/mod_target_view.h"

#include "ui/envelope_editor.h"
#include "ui/parameter_editor_view.h"

namespace sidebands {

ModulatorTargetView::ModulatorTargetView(
    const VSTGUI::CRect &size, Steinberg::Vst::EditController *edit_controller,
    TargetTag target)
    : VSTGUI::CRowColumnView(
          size, VSTGUI::CRowColumnView::kColumnStyle,
          VSTGUI::CRowColumnView::LayoutStyle::kLeftTopEqualy, 2),
      edit_controller_(edit_controller), target_(target) {
  setBackgroundColor(kBgGrey);
  int selected_generator = SelectedGenerator(edit_controller);
  VSTGUI::CRect column_size{0, 0, 40, getHeight()};

  a_slider_ = new ParameterEditorView(column_size,
                                      FindRangedParameter(edit_controller,
                                                          selected_generator,
                                                          TAG_ENV_A, target),
                                      this, "A");
  d_slider_ = new ParameterEditorView(column_size,
                                      FindRangedParameter(edit_controller,
                                                          selected_generator,
                                                          TAG_ENV_D, target),
                                      this, "D");
  s_slider_ = new ParameterEditorView(column_size,
                                      FindRangedParameter(edit_controller,
                                                          selected_generator,
                                                          TAG_ENV_S, target),
                                      this, "S");
  r_slider_ = new ParameterEditorView(column_size,
                                      FindRangedParameter(edit_controller,
                                                          selected_generator,
                                                          TAG_ENV_R, target),
                                      this, "R");

  envelope_editor_ =
      new EnvelopeEditorView({0, 0, 400, getHeight()}, this, edit_controller,
                             selected_generator, target);

  addView(a_slider_);
  addView(d_slider_);
  addView(s_slider_);
  addView(r_slider_);
  addView(envelope_editor_);
}

void ModulatorTargetView::valueChanged(VSTGUI::CControl *control) {
  Steinberg::Vst::ParamID tag = control->getTag();
  LOG(INFO) << "Value changed: " << TagStr(tag)
            << " == " << control->getValue();
  edit_controller_->beginEdit(tag);
  edit_controller_->performEdit(tag, control->getValue());
  edit_controller_->endEdit(tag);
  edit_controller_->setParamNormalized(tag, control->getValueNormalized());
}

void ModulatorTargetView::update(Steinberg::FUnknown *changedUnknown,
                                 Steinberg::int32 message) {
  if (message == IDependent::kChanged) {
    Steinberg::Vst::RangeParameter *changed_param;
    auto query_result = changedUnknown->queryInterface(
        Steinberg::Vst::RangeParameter::iid, (void **)&changed_param);
    CHECK(query_result == Steinberg::kResultOk);
    if (changed_param->getInfo().id ==
        TagFor(0, TAG_SELECTED_GENERATOR, TARGET_NA)) {
      int32_t new_generator =
          int32_t(changed_param->getNormalized() * kNumGenerators);

      envelope_editor_->SwitchGenerator(new_generator);

      a_slider_->UpdateControlParameters(
          edit_controller_, FindRangedParameter(edit_controller_, new_generator,
                                                TAG_ENV_A, target_));
      d_slider_->UpdateControlParameters(
          edit_controller_, FindRangedParameter(edit_controller_, new_generator,
                                                TAG_ENV_D, target_));
      s_slider_->UpdateControlParameters(
          edit_controller_, FindRangedParameter(edit_controller_, new_generator,
                                                TAG_ENV_S, target_));
      r_slider_->UpdateControlParameters(
          edit_controller_, FindRangedParameter(edit_controller_, new_generator,
                                                TAG_ENV_R, target_));
      setDirty(true);
    }
  }
}

void ModulatorTargetView::SwitchGenerator(int new_generator) {
  envelope_editor_->SwitchGenerator(new_generator);

  a_slider_->UpdateControlParameters(
      edit_controller_,
      FindRangedParameter(edit_controller_, new_generator, TAG_ENV_A, target_));
  d_slider_->UpdateControlParameters(
      edit_controller_,
      FindRangedParameter(edit_controller_, new_generator, TAG_ENV_D, target_));
  s_slider_->UpdateControlParameters(
      edit_controller_,
      FindRangedParameter(edit_controller_, new_generator, TAG_ENV_S, target_));
  r_slider_->UpdateControlParameters(
      edit_controller_,
      FindRangedParameter(edit_controller_, new_generator, TAG_ENV_R, target_));
}

} //  namespace sidebands
