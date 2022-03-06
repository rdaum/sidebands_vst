#include "ui/patch_param_view.h"

#include <glog/logging.h>
#include <pluginterfaces/vst/vsttypes.h>

#include <charconv>

#include "constants.h"
#include "sidebands_controller.h"

using Steinberg::Vst::ParamID;

namespace sidebands {
namespace ui {

namespace {

VSTGUI::CResourceDescription kSliderBackground("slider_rail.png");
VSTGUI::CResourceDescription kSliderHandle("slider_handle.png");
VSTGUI::CResourceDescription kSwitch("switches.png");
VSTGUI::CResourceDescription kKnob("knob2.png");
VSTGUI::CResourceDescription kSelect("select.png");
VSTGUI::CResourceDescription kOnOff("on-off.png");
VSTGUI::CResourceDescription kToggleSwitch("toggle_switch.png");

constexpr int kToggleButtonWidth = 24;

constexpr int kKnobWidth = 20;
constexpr int kKnobHeight = 20;
constexpr int kKnobSubpixmaps = 80;

class ParameterChangeListener : public Steinberg::FObject {
 public:
  ParameterChangeListener(Steinberg::Vst::Parameter *parameter,
                          VSTGUI::CControl *control)
      : parameter_(parameter), control_(control) {}

  void update(FUnknown *changedUnknown, Steinberg::int32 message) override {
    if (message != IDependent::kChanged) return;
    Steinberg::Vst::Parameter *changed_param;
    if (changedUnknown->queryInterface(Steinberg::Vst::Parameter::iid,
                                       (void **)&changed_param) !=
        Steinberg::kResultOk)
      return;

    if (changed_param != parameter_) return;

    control_->setValueNormalized(changed_param->getNormalized());
    control_->setDirty(true);
  }

 private:
  Steinberg::Vst::Parameter *parameter_;
  VSTGUI::CControl *control_;
};

}  // namespace

void PatchParameterView::RmDependent(int tag_id) {
  Steinberg::Vst::Parameter *param =
      edit_controller_->getParameterObject(tag_id);
  if (param) param->removeDependent(this);
}

VSTGUI::CControl *PatchParameterView::NewKnob(uint16_t generator,
                                              ParamTag param, TargetTag sp) {
  Steinberg::Vst::RangeParameter *ranged_parameter =
      FindRangedParameter(edit_controller(), generator, param, sp);

  auto *control =
      new VSTGUI::CAnimKnob(VSTGUI::CRect(0, 0, kKnobWidth, kKnobHeight), this,
                            ranged_parameter->getInfo().id, kKnobSubpixmaps,
                            kKnobHeight, new VSTGUI::CBitmap(kKnob));
  control->setValue(ranged_parameter->getNormalized());
  control->setMax(ranged_parameter->getMax());
  control->setMin(ranged_parameter->getMin());
  ranged_parameter->addDependent(this);

  return control;
}

VSTGUI::CControl *PatchParameterView::NewToggle(uint16_t generator,
                                                ParamTag param, TargetTag sp) {
  ParamID tag = TagFor(generator, param, sp);
  auto *param_obj = edit_controller_->getParameterObject(tag);
  auto *control = new VSTGUI::COnOffButton(
      VSTGUI::CRect(0, 0, kToggleButtonWidth, 15), this,
      TagFor(generator, param, sp), new VSTGUI::CBitmap(kToggleSwitch));
  control->setValue(param_obj->getNormalized());
  param_obj->addDependent(this);
  param_obj->addDependent(new ParameterChangeListener(param_obj, control));

  return control;
}

}  // namespace ui
}  // namespace sidebands