#include "ui/modulator_editor_view.h"

namespace sidebands {

void ModulatorEditorView::valueChanged(VSTGUI::CControl *control) {
  Steinberg::Vst::ParamID tag = control->getTag();
  edit_controller_->beginEdit(tag);
  edit_controller_->performEdit(tag, control->getValue());
  edit_controller_->endEdit(tag);
  edit_controller_->setParamNormalized(tag, control->getValueNormalized());
}

void ModulatorEditorView::update(Steinberg::FUnknown *changedUnknown,
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

      SwitchGenerator(new_generator);
    }
  }
}

}  // namespace sidebands