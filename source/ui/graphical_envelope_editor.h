#pragma once

#include <public.sdk/source/vst/vsteditcontroller.h>
#include <vstgui/vstgui.h>

#include "ui/modulator_editor_view.h"
#include "ui/patch_param_view.h"

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

  struct Segment {
    Steinberg::Vst::RangeParameter *param;
    enum class Type { RATE, LEVEL } type;
    double start_level;
    double end_level;
    double duration;

    VSTGUI::CCoord width;
    VSTGUI::CPoint start_point;
    VSTGUI::CPoint end_point;
    VSTGUI::CRect drag_box;
  };
  Steinberg::Vst::RangeParameter *a_ = nullptr;
  Steinberg::Vst::RangeParameter *d_ = nullptr;
  Steinberg::Vst::RangeParameter *s_ = nullptr;
  Steinberg::Vst::RangeParameter *r_ = nullptr;
  std::vector<Segment> segments_;
  Segment *dragging_ = nullptr;
};

}  // namespace ui
}  // namespace sidebands
