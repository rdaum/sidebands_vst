#include "controller/ui/waveform_view.h"

#include "controller/sidebands_controller.h"
#include "globals.h"
#include "processor/synthesis/oscillator.h"

namespace sidebands {
namespace ui {

WaveformView::WaveformView(const VSTGUI::CRect &size,
                           const std::vector<VSTGUI::CColor> colours,
                           const std::vector<int> &generators,
                           SidebandsController *edit_controller)
    : VSTGUI::CView(size), colours_(colours), generators_(generators),
      sidebands_controller_(edit_controller) {}

void WaveformView::draw(VSTGUI::CDrawContext *pContext) {
  return drawRect(pContext, getViewSize());
}

void WaveformView::drawRect(VSTGUI::CDrawContext *context,
                            const VSTGUI::CRect &dirtyRect) {
  VSTGUI::ConcatClip cc(*context, dirtyRect);
  if (cc.isEmpty())
    return;
  context->setClipRect(dirtyRect);
  VSTGUI::CDrawContext::Transform transform(
      *context, VSTGUI::CGraphicsTransform().translate(getViewSize().left,
                                                       getViewSize().top));

  Oscillator o;
  auto buffer_size = 1024;
  for (int gen_num = 0; gen_num < generators_.size(); gen_num++) {
    if (!sidebands_controller_->getParamNormalized(
            TagFor(gen_num, TAG_GENERATOR_TOGGLE, TARGET_NA)))
      continue;
    OscBuffer buffer(buffer_size);

    OscParam c(sidebands_controller_->GetParamValue(
                   TagFor(gen_num, TAG_OSC, TARGET_C)),
               buffer_size);
    OscParam m(sidebands_controller_->GetParamValue(
                   TagFor(gen_num, TAG_OSC, TARGET_M)),
               buffer_size);
    OscParam r(sidebands_controller_->GetParamValue(
                   TagFor(gen_num, TAG_OSC, TARGET_R)),
               buffer_size);
    OscParam s(sidebands_controller_->GetParamValue(
                   TagFor(gen_num, TAG_OSC, TARGET_S)),
               buffer_size);
    OscParam k(sidebands_controller_->GetParamValue(
                   TagFor(gen_num, TAG_OSC, TARGET_K)),
               buffer_size);
    OscParam freq(440, buffer_size);
    o.Perform(48000, buffer, freq, c, m, r, s, k);
    buffer *= 0.9;

    auto max_point = buffer.max();
    auto scale_factor = 1.0;
    if (max_point > 1) {
      scale_factor = 1 / max_point;
      LOG(INFO) << "Max point: " << max_point
                << " scale factor: " << scale_factor;
    }
    auto path = VSTGUI::owned(context->createGraphicsPath());
    if (path == nullptr)
      return;
    double mid = getHeight() / 2;
    path->beginSubpath(
        VSTGUI::CPoint(0, mid + (buffer[0] * scale_factor * getHeight() / 2)));
    for (int p = 0; p < getWidth(); p++)
      path->addLine(VSTGUI::CPoint(
          double(p), mid + (buffer[p] * scale_factor * getHeight() / 2)));

    context->setFillColor(colours_[gen_num]);
    context->setFrameColor(colours_[gen_num]);
    context->setLineStyle(VSTGUI::kLineSolid);
    context->setLineWidth(2);
    context->setDrawMode(VSTGUI::kAntiAliasing | VSTGUI::kNonIntegralMode);
    context->drawGraphicsPath(path, VSTGUI::CDrawContext::kPathStroked);

    double top_ref = 0.9;
    double bottom_ref = -0.9;

    double top_ref_line_y = mid + (top_ref * scale_factor * (getHeight() / 2));
    double bottom_ref_line_y =
        mid + (bottom_ref * scale_factor * (getHeight() / 2));
    context->drawLine({0, top_ref_line_y}, {getWidth(), top_ref_line_y});
    context->drawLine({0, bottom_ref_line_y}, {getWidth(), bottom_ref_line_y});
  }
  setDirty(false);
}

bool WaveformView::drawFocusOnTop() { return false; }

bool WaveformView::getFocusPath(VSTGUI::CGraphicsPath &outPath) {
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

void WaveformView::SetGenerators(const std::vector<int> &generators) {
  generators_ = generators;
  setDirty(true);
}

} // namespace ui
} // namespace sidebands
