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
    std::vector<std::complex<double>> buffer(buffer_size);
    std::vector<double> a(buffer_size), c(buffer_size), m(buffer_size),
        r(buffer_size), s(buffer_size), k(buffer_size /**/);

    std::fill(a.begin(), a.end(), gp->a());
    std::fill(c.begin(), c.end(), gp->c());
    std::fill(m.begin(), m.end(), gp->m());
    std::fill(r.begin(), r.end(), gp->r());
    std::fill(s.begin(), s.end(), gp->s());
    std::fill(k.begin(), k.end(), gp->k());
    o.Perform(buffer_size, 48000, buffer.data(), 440, a.data(), c.data(),
              m.data(), r.data(), s.data(), k.data());


    auto path = VSTGUI::owned(context->createGraphicsPath());
    if (path == nullptr)
      return;
    double mid = getHeight() / 2;
    path->beginSubpath(
        VSTGUI::CPoint(0, mid + (buffer[0].real() * getHeight())));
    for (int p = 0; p < buffer_size; p++)
      path->addLine(
          VSTGUI::CPoint(double(p), mid + (buffer[p].real() * getHeight())));

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
