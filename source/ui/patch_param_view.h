#pragma once

#include <public.sdk/source/vst/vsteditcontroller.h>
#include <vstgui/vstgui.h>

#include "constants.h"
#include "tags.h"

namespace sidebands {

class SidebandsController;

class PatchParameterView : public VSTGUI::IControlListener,
                           public Steinberg::FObject {
public:
  explicit PatchParameterView(SidebandsController *edit_controller)
      : edit_controller_(edit_controller) {}

protected:
  SidebandsController *edit_controller() const { return edit_controller_; }
  void RmDependent(int tag_id);

  VSTGUI::CControl *NewKnob(uint16_t generator, ParamTag param, TargetTag sp);
  VSTGUI::CControl *NewToggle(uint16_t generator, ParamTag param, TargetTag sp);

private:
  SidebandsController *edit_controller_;
};
} // namespace sidebands