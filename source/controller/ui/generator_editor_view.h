#pragma once

#include <public.sdk/source/vst/vsteditcontroller.h>
#include <vstgui/vstgui.h>

#include "globals.h"
#include "constants.h"
#include "tags.h"
#include "controller/ui/patch_param_view.h"

namespace sidebands {
class SidebandsController;

namespace ui {

class ParameterEditorView;
class GraphicalEnvelopeEditorView;
class ModulatorTargetView;
class WaveformView;
class SpectrumView;
class GeneratorEditorView : public VSTGUI::CScrollView, Steinberg::FObject {
public:
  GeneratorEditorView(const VSTGUI::CRect &size,
                      SidebandsController *edit_controller);
  ~GeneratorEditorView() override;
  void HighlightEnvelopeStage(TargetTag target, off_t stage);

  // IControlListener overrides
  void valueChanged(VSTGUI::CControl *control) override;

  // IDependent overrides
  void update(FUnknown *unknown, Steinberg::int32 int_32) override;


private:
  SidebandsController *edit_controller_;
  VSTGUI::CTextLabel *selected_label_;
  std::vector<ParameterEditorView*> parameter_editors_;
  VSTGUI::COnOffButton *analog_mode_;

  WaveformView *waveform_view_;
  SpectrumView *spectrum_view_;

  ModulatorTargetView *a_target_view_;
  ModulatorTargetView *k_target_view_;
};

}  // namespace ui
}  // namespace sidebands