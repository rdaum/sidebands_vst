#pragma once

#include <public.sdk/source/vst/vsteditcontroller.h>
#include <vstgui/vstgui.h>

#include "constants.h"
#include "tags.h"

namespace sidebands {

class SidebandsController;

namespace ui {

class PatchParameterView : public VSTGUI::IControlListener,
                           public Steinberg::FObject {
 public:
  explicit PatchParameterView(SidebandsController *edit_controller)
      : edit_controller_(edit_controller) {}

 protected:
  SidebandsController *edit_controller() const { return edit_controller_; }

 private:
  SidebandsController *edit_controller_;
};

}  // namespace ui
}  // namespace sidebands