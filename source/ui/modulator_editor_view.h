#pragma once

#include <public.sdk/source/vst/vsteditcontroller.h>
#include <vstgui/vstgui.h>

#include "constants.h"
#include "tags.h"

namespace sidebands {
namespace ui {

// Abstract mixin for modulation source editors (envelopes, LFOs)
class ModulatorEditorView : public Steinberg::FObject,
                            public VSTGUI::IControlListener {
public:
  ModulatorEditorView(Steinberg::Vst::EditController *edit_controller,
                      TargetTag target)
      : edit_controller_(edit_controller), target_(target) {}
  virtual void SwitchGenerator(int new_generator) = 0;

  // IControlListener overrides
  void valueChanged(VSTGUI::CControl *control) override;

  // IDependent overrides
  virtual void update(FUnknown *unknown, Steinberg::int32 int_32) override;

  Steinberg::Vst::EditController *edit_controller() const {
    return edit_controller_;
  }
  TargetTag target() const { return target_; }

private:
  Steinberg::Vst::EditController *edit_controller_;
  const TargetTag target_;
};

}  // namespace ui
}  // namespace sidebands