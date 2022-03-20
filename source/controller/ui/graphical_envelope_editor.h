#pragma once

#include <public.sdk/source/vst/vsteditcontroller.h>
#include <vstgui/vstgui.h>

#include "globals.h"
#include "controller/ui/modulator_editor_view.h"
#include "controller/ui/patch_param_view.h"

namespace sidebands {
namespace ui {

class GraphicalEnvelopeEditorView : public VSTGUI::CView,
                                    public VSTGUI::IFocusDrawing,
                                    public ModulatorEditorView {
 public:
  GraphicalEnvelopeEditorView(const VSTGUI::CRect &size,
                              VSTGUI::IControlListener *listener,
                              SidebandsController *edit_controller,
                              int selected_generator, TargetTag target);

  void SwitchGenerator(int new_generator) override;
  void RefreshState(const PlayerState::VoiceState::GeneratorState::ModulationState &player_state);

  // IDependent overrides
  void update(FUnknown *unknown, Steinberg::int32 int_32) override;

  // CView overrides
  void draw(VSTGUI::CDrawContext *pContext) override;
  void drawRect(VSTGUI::CDrawContext *context,
                const VSTGUI::CRect &dirtyRect) override;
  VSTGUI::CMouseEventResult onMouseDown(
      VSTGUI::CPoint &where, const VSTGUI::CButtonState &buttons) override;
  void setDirty(bool val) override;
  VSTGUI::CMouseEventResult onMouseMoved(
      VSTGUI::CPoint &where, const VSTGUI::CButtonState &buttons) override;
  VSTGUI::CMouseEventResult onMouseUp(
      VSTGUI::CPoint &where, const VSTGUI::CButtonState &buttons) override;

  // IFocusDrawing overrides
  bool drawFocusOnTop() override;
  bool getFocusPath(VSTGUI::CGraphicsPath &outPath) override;

 private:
  void UpdateSegments();

  Steinberg::Vst::RangeParameter *Param(uint16_t generator, ParamTag param);
  Steinberg::Vst::RangeParameter *Param(ParamTag param);

  struct Segment {
    Steinberg::Vst::RangeParameter *rate_param;
    Steinberg::Vst::RangeParameter *start_level_param;
    Steinberg::Vst::RangeParameter *end_level_param;

    VSTGUI::CCoord width;
    VSTGUI::CPoint start_point;
    VSTGUI::CPoint end_point;
    VSTGUI::CRect drag_box;
  };

  std::vector<Steinberg::Vst::RangeParameter*> envelope_parameters;
  std::vector<Segment> segments_;
  Segment *dragging_segment_ = nullptr;
  off_t playing_segment_ = 0;
};

}  // namespace ui
}  // namespace sidebands
