#pragma once

#include <public.sdk/source/vst/vsteditcontroller.h>
#include <vstgui/vstgui.h>

#include "constants.h"
#include "tags.h"
#include "ui/patch_param_view.h"

namespace sidebands {

class SidebandsController;
namespace ui {

class DrawbarView : public VSTGUI::CRowColumnView, public PatchParameterView {
public:
  explicit DrawbarView(const VSTGUI::CRect &size,
                       SidebandsController *edit_controller);

  // IControlListener overrides
  void valueChanged(VSTGUI::CControl *control) override;

private:
  VSTGUI::COnOffButton *select_buttons_[kNumGenerators];
};

}  // namespace ui
} // namespace sidebands