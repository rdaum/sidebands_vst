#include "ui/graphical_envelope_editor.h"

#include <glog/logging.h>

using VSTGUI::CPoint;

namespace sidebands {
namespace ui {

Steinberg::Vst::ParamValue ValueOf(Steinberg::Vst::RangeParameter *p) {
  return p->toPlain(p->getNormalized());
}

GraphicalEnvelopeEditorView::GraphicalEnvelopeEditorView(
    const VSTGUI::CRect &size, VSTGUI::IControlListener *listener,
    Steinberg::Vst::EditController *edit_controller, int selected_generator,
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
  segments_ = {{a_, Segment::Type::RATE, 0, 1, ValueOf(a_)},
               {d_, Segment::Type::RATE, 1, ValueOf(s_), ValueOf(d_)},
               {s_, Segment::Type::LEVEL, ValueOf(s_), ValueOf(s_),
                0.5f /* fixed sustain duration */},
               {r_, Segment::Type::RATE, ValueOf(s_), 0, ValueOf(r_)}};

  double total_duration = 0.0f;
  double min_duration = 0.1f;
  for (auto s : segments_) {
    total_duration += s.duration + min_duration;
  }

  double xpos = getViewSize().left;
  double bottom = getViewSize().bottom;

  double width = getWidth();
  double height = getHeight();

  for (auto &s : segments_) {
    s.width = ((s.duration + min_duration) / total_duration) * width;

    float yleft = bottom - (s.start_level * height);
    float yright = bottom - (s.end_level * height);

    s.start_point = {xpos, yleft};
    xpos += s.width;
    s.end_point = {xpos, yright};

    if (s.type == Segment::Type::RATE)
      s.drag_box = {s.end_point.x - 5, s.end_point.y - 5, s.end_point.x + 5,
                    s.end_point.y + 5};
    else {
      auto x = s.start_point.x + ((s.end_point.x - s.start_point.x) / 2);

      s.drag_box = {x - 5, s.end_point.y - 5, x + 5, s.end_point.y + 5};
    }
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
  context->setLineWidth(5);
  context->setDrawMode(VSTGUI::kAntiAliasing | VSTGUI::kNonIntegralMode);
  auto path = VSTGUI::owned(context->createGraphicsPath());
  if (path == nullptr)
    return;
  path->beginSubpath(getViewSize().getBottomLeft());
  for (auto &s : segments_) {
    path->addLine(s.start_point);
    path->addLine(s.end_point);
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
        getFrame()->setCursor(s.type == Segment::Type::RATE
                                  ? VSTGUI::kCursorHSize
                                  : VSTGUI::kCursorVSize);
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

  if (dragging_ && dragging_->type == Segment::Type::RATE) {
    float change_x = where.x - dragging_->end_point.x;
    float change_r = change_x / dragging_->width;
    dragging_->param->setNormalized(
        std::max(dragging_->param->getNormalized() * change_r, 0.01));
    UpdateSegments();
    setDirty(true);
    return VSTGUI::kMouseEventHandled;
  }

  if (dragging_ && dragging_->type == Segment::Type::LEVEL) {
    float change_n = 1.0 - (where.y / getHeight());
    LOG(INFO) << "Change level: " << change_n;
    dragging_->param->setNormalized(change_n);
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
  if (a_)
    a_->removeDependent(this);
  if (d_)
    d_->removeDependent(this);
  if (s_)
    s_->removeDependent(this);
  if (r_)
    r_->removeDependent(this);

  a_ = FindRangedParameter(edit_controller(), new_generator, TAG_ENV_A,
                           target());
  d_ = FindRangedParameter(edit_controller(), new_generator, TAG_ENV_D,
                           target());
  s_ = FindRangedParameter(edit_controller(), new_generator, TAG_ENV_S,
                           target());
  r_ = FindRangedParameter(edit_controller(), new_generator, TAG_ENV_R,
                           target());
  a_->addDependent(this);
  d_->addDependent(this);
  s_->addDependent(this);
  r_->addDependent(this);

  UpdateSegments();
  setDirty(true);
}

}  // namespace ui
}  // namespace sidebands