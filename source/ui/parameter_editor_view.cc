#include "ui/parameter_editor_view.h"

#include <charconv>
#include <glog/logging.h>
#include <pluginterfaces/vst/vsttypes.h>

namespace sidebands {
namespace ui {

namespace {

constexpr int kSliderWidth = 24;
constexpr int kSliderHeight = 249;
constexpr int kSliderMaxPos = 235;
constexpr int kNumericEditHeight = 15;

VSTGUI::CResourceDescription kSliderBackground("slider_rail.png");
VSTGUI::CResourceDescription kSliderHandle("slider_handle.png");

VSTGUI::CControl *
MakeVerticalSlider(Steinberg::Vst::RangeParameter *range_parameter,
                   VSTGUI::IControlListener *listener) {

  auto *slider_control = new VSTGUI::CVerticalSlider(
      VSTGUI::CRect(0, 0, kSliderWidth, kSliderHeight), listener,
      range_parameter->getInfo().id, 0, kSliderMaxPos,
      new VSTGUI::CBitmap(kSliderHandle),
      new VSTGUI::CBitmap(kSliderBackground));
  slider_control->setBackColor(kBgGrey);
  slider_control->setBackgroundOffset(VSTGUI::CPoint(-7.5, 0));
  slider_control->setOffsetHandle(VSTGUI::CPoint(4, 0));
  slider_control->setMax(range_parameter->getMax());
  slider_control->setMin(range_parameter->getMin());
  slider_control->setBackColor(kBgGrey);
  slider_control->setValueNormalized(range_parameter->getNormalized());

  return slider_control;
}

VSTGUI::CControl *
MakeNumericEditor(Steinberg::Vst::RangeParameter *range_parameter,
                  VSTGUI::IControlListener *listener) {

  auto *edit_control = new VSTGUI::CTextEdit(
      VSTGUI::CRect(0, 0, kSliderWidth, kNumericEditHeight), listener,
      range_parameter->getInfo().id);
  edit_control->setMax(range_parameter->getMax());
  edit_control->setMin(range_parameter->getMin());
  edit_control->setValueNormalized(range_parameter->getNormalized());
  VSTGUI::CTextEdit::StringToValueFunction str_function =
      [](VSTGUI::UTF8StringPtr txt, float& result, VSTGUI::CTextEdit* textEdit) -> bool {
         try {
           float v = std::stof(txt);
           result = v;
           return true;
         } catch (const std::exception &e) {
           return false;
         }
      };
  edit_control->setStringToValueFunction(str_function);

  return edit_control;
}

} // namespace

ParameterEditorView::ParameterEditorView(
    const VSTGUI::CRect &size, Steinberg::Vst::RangeParameter *parameter,
    VSTGUI::IControlListener *listener, const std::string &label)
    : VSTGUI::CRowColumnView(
          size, VSTGUI::CRowColumnView::kRowStyle,
          VSTGUI::CRowColumnView::LayoutStyle::kLeftTopEqualy, 2),
      parameter_(parameter) {
  parameter->addDependent(this);

  controls_ = {MakeVerticalSlider(parameter, listener),
               MakeNumericEditor(parameter, listener)};
  if (!label.empty())
    addView(new VSTGUI::CTextLabel(
        VSTGUI::CRect(0, 0, getWidth(), kNumericEditHeight), label.c_str()));
  for (auto *item : controls_) {
    addView(item);
  }
}

ParameterEditorView::~ParameterEditorView() {
  parameter_->removeDependent(this);
}

void ParameterEditorView::update(Steinberg::FUnknown *unknown,
                                 Steinberg::int32 message) {

  if (message != IDependent::kChanged)
    return;
  Steinberg::Vst::Parameter *changed_param;
  if (unknown->queryInterface(Steinberg::Vst::Parameter::iid,
                              (void **)&changed_param) != Steinberg::kResultOk)
    return;

  if (changed_param != parameter_)
    return;

  // Make sure all values are updated.
  for (auto *control : controls_) {
    control->setValueNormalized(changed_param->getNormalized());
    control->setDirty(true);
  }
}

void ParameterEditorView::UpdateControlParameters(
    Steinberg::Vst::EditController *edit_controller,
    Steinberg::Vst::RangeParameter *new_parameter) {
  parameter_->removeDependent(this);
  for (auto *control : controls_) {
    Steinberg::Vst::ParamID old_tag = control->getTag();
    auto *old_param_obj = edit_controller->getParameterObject(old_tag);
    if (!old_param_obj) {
      LOG(ERROR) << "Missing parameter for tag: " << TagStr(old_tag);
      continue;
    }

    Steinberg::Vst::ParamID new_tag = new_parameter->getInfo().id;
    control->setTag(new_tag);
    control->setValueNormalized(new_parameter->getNormalized());
    control->setDirty(true);
  }
  new_parameter->addDependent(this);
  parameter_ = new_parameter;
  setDirty(true);
}

}  // namespace ui
}  // namespace sidebands
