#include "ui/waveform_view.h"

#include "globals.h"
#include "synthesis/oscillator.h"
#include "synthesis/patch.h"

namespace sidebands {
namespace ui {

WaveformView::WaveformView(const VSTGUI::CRect &size,
                           const std::vector<VSTGUI::CColor> colours,
                           const std::vector<int> &generators)
    : VSTGUI::CView(size), colours_(colours), generators_(generators) {}

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
  auto buffer_size = size_t(getWidth());
  for (int gen_num = 0; gen_num < generators_.size(); gen_num++) {
    auto &gp = kPatch->generators_[gen_num];
    if (!gp->on())
      continue;
    OscBuffer buffer(buffer_size);

    Oscillator::OscParam a(gp->a(), buffer_size);
    Oscillator::OscParam c(gp->c(), buffer_size);
    Oscillator::OscParam m(gp->m(), buffer_size);
    Oscillator::OscParam r(gp->r(), buffer_size);
    Oscillator::OscParam s(gp->s(), buffer_size);
    Oscillator::OscParam k(std::complex<double>(0,gp->k()), buffer_size);
    Oscillator::OscParam freq(84, buffer_size);
    o.Perform(48000, buffer, freq,  c,  m, r, s, k);

    std::valarray<double> realized(buffer_size);
    for (int i = 0; i < buffer_size; i++) {
      realized[i] = buffer[i].real();
    }
    auto path = VSTGUI::owned(context->createGraphicsPath());
    if (path == nullptr)
      return;
    double mid = getHeight() / 2;
    path->beginSubpath(
        VSTGUI::CPoint(0, mid + (realized[0] * getHeight() / 2)));
    for (int p = 0; p < buffer_size; p++)
      path->addLine(
          VSTGUI::CPoint(double(p), mid + (realized[p] * getHeight() / 2)));

    context->setFillColor(colours_[gen_num]);
    context->setFrameColor(colours_[gen_num]);
    context->setLineStyle(VSTGUI::kLineSolid);
    context->setLineWidth(2);
    context->setDrawMode(VSTGUI::kAntiAliasing | VSTGUI::kNonIntegralMode);
    context->drawGraphicsPath(path, VSTGUI::CDrawContext::kPathStroked);
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
