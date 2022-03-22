#include "controller/ui/parameter_editor_view.h"

#include <glog/logging.h>
#include <pluginterfaces/vst/vsttypes.h>

#include <charconv>

#include "controller/sidebands_controller.h"
#include "controller/ui/gui_constants.h"

namespace sidebands {
namespace ui {

namespace {

VSTGUI::CControl *MakeSlider(Steinberg::Vst::RangeParameter *range_parameter,
                   VSTGUI::IControlListener *listener,
                   ParameterEditorStyle style) {
  SliderDesc slider_desc;
  switch (style) {
  case ParameterEditorStyle::SLIDER_VERTICAL_TALL:
    slider_desc = kVerticalSliderTallDimensions;
    break;
  case ParameterEditorStyle::SLIDER_VERTICAL_SHORT:
    slider_desc = kVerticalSliderShortDimensions;
    break;
  case ParameterEditorStyle::SLIDER_HORIZONTAL_SHORT:
    slider_desc = kHorizontalSliderShortDimensions;
    break;
  }

  VSTGUI::CSlider *slider_control;
  if (style == ParameterEditorStyle::SLIDER_HORIZONTAL_SHORT) {
    slider_control = new VSTGUI::CHorizontalSlider(
        VSTGUI::CRect(0, 0, slider_desc.width, slider_desc.height), listener,
        range_parameter->getInfo().id, 0, slider_desc.max_pos,
        new VSTGUI::CBitmap(slider_desc.handle),
        new VSTGUI::CBitmap(slider_desc.background), {0,0}, VSTGUI::CSlider::kLeft);
    slider_control->setBackgroundOffset(VSTGUI::CPoint(0, -7.5));
    slider_control->setOffsetHandle(VSTGUI::CPoint(0, 4));

  } else {
    slider_control = new VSTGUI::CVerticalSlider(
        VSTGUI::CRect(0, 0, slider_desc.width, slider_desc.height), listener,
        range_parameter->getInfo().id, 0, slider_desc.max_pos,
        new VSTGUI::CBitmap(slider_desc.handle),
        new VSTGUI::CBitmap(slider_desc.background));
    slider_control->setBackgroundOffset(VSTGUI::CPoint(-7.5, 0));
    slider_control->setOffsetHandle(VSTGUI::CPoint(4, 0));
  }
  slider_control->setMax(range_parameter->getMax());
  slider_control->setMin(range_parameter->getMin());
  slider_control->setValueNormalized(range_parameter->getNormalized());
  slider_control->setBackColor(VSTGUI::kTransparentCColor);
  return slider_control;
}

VSTGUI::CControl *
MakeNumericEditor(Steinberg::Vst::RangeParameter *range_parameter,
                  VSTGUI::IControlListener *listener) {

  auto *edit_control = new VSTGUI::CTextEdit(
      VSTGUI::CRect(0, 0, 40, kNumericEditHeight), listener,
      range_parameter->getInfo().id);
  edit_control->setMax(range_parameter->getMax());
  edit_control->setMin(range_parameter->getMin());
  edit_control->setValueNormalized(range_parameter->getNormalized());
  edit_control->setBackColor(VSTGUI::kBlackCColor);
  edit_control->setFontColor(VSTGUI::kWhiteCColor);
  edit_control->setFrameWidth(0);
  edit_control->setFont(VSTGUI::kNormalFontSmall);

  VSTGUI::CTextEdit::StringToValueFunction str_function =
      [](VSTGUI::UTF8StringPtr txt, float &result,
         VSTGUI::CTextEdit *textEdit) -> bool {
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

VSTGUI::CControl *
MakeKnob(Steinberg::Vst::RangeParameter *range_parameter,
                  VSTGUI::IControlListener *listener) {

  auto *edit_control = new VSTGUI::CKnob(
      VSTGUI::CRect(0, 0, 40, 40), listener,
      range_parameter->getInfo().id, new VSTGUI::CBitmap(kKnobBase), nullptr, {0,0}, VSTGUI::CKnob::kHandleCircleDrawing);
  edit_control->setMax(range_parameter->getMax());
  edit_control->setMin(range_parameter->getMin());
  edit_control->setHandleLineWidth(2);
  edit_control->setValueNormalized(range_parameter->getNormalized());

  return edit_control;
}

} // namespace

ParameterEditorView::ParameterEditorView(
    const VSTGUI::CRect &size, Steinberg::Vst::RangeParameter *parameter,
    VSTGUI::IControlListener *listener, ParameterEditorStyle style,
    const std::string &label)
    : VSTGUI::CRowColumnView(
          size, VSTGUI::CRowColumnView::kRowStyle,
          VSTGUI::CRowColumnView::LayoutStyle::kLeftTopEqualy, 2),
      parameter_(parameter) {
  setBackgroundColor(VSTGUI::kTransparentCColor);

  parameter->addDependent(this);

  if (style != ParameterEditorStyle::NUMERIC_ENTRY && style != ParameterEditorStyle::KNOB) {
    controls_ = {MakeSlider(parameter, listener, style)};
  }
  if (style == ParameterEditorStyle::KNOB) {
    controls_ = { MakeKnob(parameter, listener)};
  }
  controls_.push_back(MakeNumericEditor(parameter, listener));

  if (!label.empty()) {
    VSTGUI::CTextLabel *column_label = new VSTGUI::CTextLabel(
        VSTGUI::CRect(0, 0, getWidth(), kColumnLabelHeight), label.c_str());
    column_label->setFont(VSTGUI::kNormalFont);
    column_label->setBackColor(VSTGUI::kTransparentCColor);
    column_label->setFontColor(VSTGUI::kBlackCColor);
    column_label->setFrameWidth(0);
    addView(column_label);
  }
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
    SidebandsController *edit_controller,
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

TargetTag ParameterEditorView::target() const {
  return TargetFor(parameter_->getInfo().id);
}

ParamTag ParameterEditorView::tag() const {
  return ParamFor(parameter_->getInfo().id);
}

} // namespace ui
} // namespace sidebands
