#include "controller/ui/graphical_envelope_editor.h"

#include <glog/logging.h>

#include "controller/sidebands_controller.h"
#include "globals.h"

using VSTGUI::CPoint;

namespace sidebands {
namespace ui {

constexpr double kSustainDuration = 0.10;

Steinberg::Vst::ParamValue ValueOf(Steinberg::Vst::RangeParameter *p) {
  return p ? p->toPlain(p->getNormalized()) : 0;
}

GraphicalEnvelopeEditorView::GraphicalEnvelopeEditorView(
    const VSTGUI::CRect &size, VSTGUI::IControlListener *listener,
    SidebandsController *edit_controller, int selected_generator,
    TargetTag target)
    : VSTGUI::CView(size), ModulatorEditorView(edit_controller, target) {
  SwitchGenerator(selected_generator);
}

void GraphicalEnvelopeEditorView::update(Steinberg::FUnknown *unknown,
                                         Steinberg::int32 int_32) {
  ModulatorEditorView::update(unknown, int_32);
  FObject::update(unknown, int_32);
  UpdateSegments();
  setDirty(true);
}

void GraphicalEnvelopeEditorView::UpdateSegments() {
  segments_ = {
      Segment{Param(TAG_ENV_HT), nullptr, nullptr},
      Segment{Param(TAG_ENV_AR), nullptr, Param(TAG_ENV_AL)},
      Segment{Param(TAG_ENV_DR1), Param(TAG_ENV_AL), Param(TAG_ENV_DL1)},
      Segment{Param(TAG_ENV_DR2), Param(TAG_ENV_DL1), Param(TAG_ENV_SL)},
      Segment{nullptr, Param(TAG_ENV_SL), Param(TAG_ENV_SL)},
      Segment{Param(TAG_ENV_RR1), Param(TAG_ENV_SL), Param(TAG_ENV_RL1)},
      Segment{Param(TAG_ENV_RR2), Param(TAG_ENV_RL1), nullptr},
  };

  double total_duration = 0.0f;
  for (auto s : segments_) {
    auto duration = s.rate_param ? ValueOf(s.rate_param) : 0;

    total_duration += duration;
  }
  total_duration += kSustainDuration;

  double xpos = getViewSize().left;
  double bottom = getViewSize().bottom;

  double width = getWidth();
  double height = getHeight();

  for (auto &s : segments_) {
    bool is_sustain = s.start_level_param &&
                      ParamFor(s.start_level_param->getInfo().id) == TAG_ENV_SL;
    auto duration = is_sustain ? kSustainDuration : ValueOf(s.rate_param);
    s.width = (((duration)) / total_duration) * width;

    auto start_level = ValueOf(s.start_level_param);
    auto end_level = ValueOf(s.end_level_param);
    float yleft = bottom - (start_level * height);
    float yright = bottom - (end_level * height);

    s.start_point = {xpos, yleft};
    xpos += s.width;
    s.end_point = {xpos, yright};

    s.drag_box = {s.end_point.x - 5, s.end_point.y - 5, s.end_point.x + 5,
                  s.end_point.y + 5};
  }
}

void GraphicalEnvelopeEditorView::draw(VSTGUI::CDrawContext *pContext) {
  return drawRect(pContext, getViewSize());
}

void GraphicalEnvelopeEditorView::setDirty(bool val) {
  CView::setDirty(val);
  UpdateSegments();
}

void GraphicalEnvelopeEditorView::drawRect(VSTGUI::CDrawContext *context,
                                           const VSTGUI::CRect &dirtyRect) {
  if (dirtyRect.getWidth() <= 0 || dirtyRect.getHeight() <= 0 ||
      context == nullptr)
    return;

  context->setFrameColor(VSTGUI::CColor(100, 0, 0));
  context->setFillColor(VSTGUI::CColor(100, 0, 0));
  context->setLineStyle(VSTGUI::kLineSolid);
  context->setLineWidth(1);
  context->setDrawMode(VSTGUI::kAntiAliasing | VSTGUI::kNonIntegralMode);
  auto path = VSTGUI::owned(context->createGraphicsPath());
  if (path == nullptr)
    return;
  const auto bottom_left = getViewSize().getBottomLeft();

  path->beginSubpath(bottom_left);
  VSTGUI::CCoord x = 0;
  for (auto &s : segments_) {
    double level = ValueOf(s.start_level_param);
    if (level == 0)
      level = 0.001;
    double end_level = ValueOf(s.end_level_param);
    if (end_level == 0)
      end_level = 0.001;
    auto co = EnvelopeRampCoefficient(level, end_level, s.width);
    for (int i = 0; i < s.width; i++) {
      level *= co;
      double height = level * getViewSize().getHeight();
      path->addLine(bottom_left.x + (x++), bottom_left.y - height);
    }
  }
  context->drawGraphicsPath(path, VSTGUI::CDrawContext::kPathStroked);

  context->setFrameColor(VSTGUI::CColor(0, 0, 100));
  context->setLineWidth(1);
  for (auto &s : segments_) {
    context->drawRect(s.drag_box);
  }
  setDirty(false);
}

VSTGUI::CMouseEventResult
GraphicalEnvelopeEditorView::onMouseDown(CPoint &where,
                                         const VSTGUI::CButtonState &buttons) {
  if (buttons.isLeftButton() && !dragging_)
    for (auto &s : segments_) {
      if (s.drag_box.pointInside(where)) {
        dragging_ = &s;
        return VSTGUI::kMouseEventHandled;
      }
    }

  return VSTGUI::kMouseEventNotHandled;
}

VSTGUI::CMouseEventResult
GraphicalEnvelopeEditorView::onMouseMoved(CPoint &where,
                                          const VSTGUI::CButtonState &buttons) {
  if (!dragging_ || !buttons.isLeftButton())
    return VSTGUI::kMouseEventNotHandled;

  if (dragging_) {
    float change_x = where.x - dragging_->end_point.x;
    float change_n = 1.0 - (where.y / getHeight());
    float change_r = change_x / dragging_->width;
    if (dragging_->rate_param)
      dragging_->rate_param->setNormalized(
          std::max(dragging_->rate_param->getNormalized() * change_r, 0.01));
    if (dragging_->end_level_param)
      dragging_->end_level_param->setNormalized(change_n);

    UpdateSegments();
    setDirty(true);
    return VSTGUI::kMouseEventHandled;
  }

  return VSTGUI::kMouseEventNotHandled;
}

VSTGUI::CMouseEventResult
GraphicalEnvelopeEditorView::onMouseUp(CPoint &where,
                                       const VSTGUI::CButtonState &buttons) {
  if (buttons.isLeftButton() && dragging_) {
    dragging_ = nullptr;
    getFrame()->setCursor(VSTGUI::kCursorDefault);
    return VSTGUI::kMouseEventHandled;
  }

  return VSTGUI::kMouseEventNotHandled;
}

bool GraphicalEnvelopeEditorView::drawFocusOnTop() { return false; }

bool GraphicalEnvelopeEditorView::getFocusPath(VSTGUI::CGraphicsPath &outPath) {
  if (wantsFocus()) {
    VSTGUI::CCoord focusWidth = getFrame()->getFocusWidth();
    VSTGUI::CRect r(getVisibleViewSize());
    if (!r.isEmpty()) {
      outPath.addRect(r);
      r.extend(focusWidth, focusWidth);
      outPath.addRect(r);
    }
  }
  return true;
}

void GraphicalEnvelopeEditorView::SwitchGenerator(int new_generator) {
  for (auto *param : envelope_parameters) {
    if (param)
      param->removeDependent(this);
  }
  envelope_parameters = {
      Param(new_generator, TAG_ENV_HT),  Param(new_generator, TAG_ENV_AR),
      Param(new_generator, TAG_ENV_DR1), Param(new_generator, TAG_ENV_DR2),
      Param(new_generator, TAG_ENV_RR1), Param(new_generator, TAG_ENV_RR2),
      Param(new_generator, TAG_ENV_AL),  Param(new_generator, TAG_ENV_DL1),
      Param(new_generator, TAG_ENV_SL),  Param(new_generator, TAG_ENV_RL1),
  };
  for (auto *param : envelope_parameters) {
    param->addDependent(this);
  }
  UpdateSegments();

  setDirty(true);
}

Steinberg::Vst::RangeParameter *
GraphicalEnvelopeEditorView::Param(uint16_t generator, ParamTag param) {
  return edit_controller()->FindRangedParameter(generator, param, target());
}

Steinberg::Vst::RangeParameter *
GraphicalEnvelopeEditorView::Param(ParamTag param) {
  return Param(edit_controller()->SelectedGenerator(), param);
}

} // namespace ui
} // namespace sidebands