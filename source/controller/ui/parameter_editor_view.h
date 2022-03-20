#pragma once

#include <public.sdk/source/vst/vsteditcontroller.h>
#include <vstgui/vstgui.h>

#include "constants.h"
#include "tags.h"

namespace sidebands {

class SidebandsController;

namespace ui {

enum class ParameterEditorStyle {
  VERTICAL_TALL,
  VERTICAL_SHORT,
  HORIZONTAL_SHORT
};

// Combination of a slider and numeric text editor and possibly other things.
// Tied to a single parameter and updating itself in response to parameter
// changes.
class ParameterEditorView : public VSTGUI::CRowColumnView,
                            public Steinberg::FObject {
 public:
  ParameterEditorView(const VSTGUI::CRect &size,
                      Steinberg::Vst::RangeParameter *parameter,
                      VSTGUI::IControlListener *listener,
                       ParameterEditorStyle style = ParameterEditorStyle::VERTICAL_TALL,
                      const std::string &label = "");
  ~ParameterEditorView() override;

  // Change the parameter associated with all controls (e.g. for generator
  // change) and update them to reflect the new values.
  void UpdateControlParameters(SidebandsController *edit_controller,
                               Steinberg::Vst::RangeParameter *new_parameter);

  // IDependent overrides
  void update(Steinberg::FUnknown *unknown, Steinberg::int32 int_32) override;

  TargetTag target() const;
  ParamTag tag() const;

 private:
  std::vector<VSTGUI::CControl *> controls_;
  Steinberg::Vst::RangeParameter *parameter_;
};

}  // namespace ui
}  // namespace sidebands
