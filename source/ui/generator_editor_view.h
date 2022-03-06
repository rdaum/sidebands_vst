#pragma once

#include <public.sdk/source/vst/vsteditcontroller.h>
#include <vstgui/vstgui.h>

#include "constants.h"
#include "tags.h"
#include "ui/patch_param_view.h"

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

  // IControlListener overrides
  void valueChanged(VSTGUI::CControl *control) override;

  // IDependent overrides
  void update(FUnknown *unknown, Steinberg::int32 int_32) override;

 private:
  SidebandsController *edit_controller_;
  VSTGUI::CTextLabel *selected_label_;
  ParameterEditorView *c_slider_;
  ParameterEditorView *m_slider_;
  ParameterEditorView *k_slider_;
  ParameterEditorView *r_slider_;  // TODO a dial, not slider
  ParameterEditorView *s_slider_;

  WaveformView *waveform_view_;
  SpectrumView *spectrum_view_;

  ModulatorTargetView *a_target_view_;
  ModulatorTargetView *k_target_view_;
};

}  // namespace ui
}  // namespace sidebands