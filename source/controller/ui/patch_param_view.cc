#include "controller/ui/patch_param_view.h"

#include <glog/logging.h>
#include <pluginterfaces/vst/vsttypes.h>

#include <charconv>

#include "constants.h"
#include "controller/sidebands_controller.h"
#include "controller/ui/gui_constants.h"

using Steinberg::Vst::ParamID;

namespace sidebands {
namespace ui {

namespace {

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

}  // namespace ui
}  // namespace sidebands