#include "ui/spectrum_view.h"

#include <numbers>
#include <valarray>

#include "globals.h"
#include "synthesis/oscillator.h"
#include "synthesis/patch.h"

namespace sidebands {
namespace ui {

using Complex = std::complex<double>;
using CArray = std::valarray<Complex>;

namespace {
// Cooley–Tukey FFT (in-place, divide-and-conquer)
// Higher memory requirements and redundancy although more intuitive
void FFT(CArray &x) {
  const size_t N = x.size();
  if (N <= 1)
    return;

  // divide
  CArray even = x[std::slice(0, N / 2, 2)];
  CArray odd = x[std::slice(1, N / 2, 2)];

  // conquer
  FFT(even);
  FFT(odd);

  // combine
  for (size_t k = 0; k < N / 2; ++k) {
    Complex t = std::polar(1.0, -2 * std::numbers::pi * k / N) * odd[k];
    x[k] = even[k] + t;
    x[k + N / 2] = even[k] - t;
  }
}

void HanningWindow(CArray &buf) {
  auto samples_minus_1 = buf.size() - 1;
  for (int i = 0; i < buf.size(); i++) {
    auto multiplier = 0.5 * (1 - cos(2*std::numbers::pi*i/ samples_minus_1));
    buf[i] = multiplier * buf[i];
  }
}

void HammingWindow(CArray &buf) {
  double samples_minus_1 = buf.size() - 1;
  for (int i = 0; i < buf.size(); i++) {
    auto multiplier = 0.54 - (0.46 * std::cos(2 * std::numbers::pi * (double(i) / samples_minus_1)));
    buf[i] = multiplier * buf[i];
  }
}

void BlackingWindow(CArray &buf) {
  double samples_minus_1 = buf.size() - 1;

  for (int i = 0; i < buf.size(); i++) {
    auto multiplier =
        0.42 -
        (0.5 * cos(2. * std::numbers::pi * (double(i) / samples_minus_1))) +
        (0.08 * cos(4. * M_PI * (double(i) / samples_minus_1)));
    buf[i] *= multiplier;
  }
}
} // namespace

SpectrumView::SpectrumView(const VSTGUI::CRect &size,
                           const std::vector<VSTGUI::CColor> colours,
                           const std::vector<int> &generators)
    : VSTGUI::CView(size), colours_(colours), generators_(generators) {}

void SpectrumView::draw(VSTGUI::CDrawContext *pContext) {
  return drawRect(pContext, getViewSize());
}

void SpectrumView::drawRect(VSTGUI::CDrawContext *context,
                            const VSTGUI::CRect &dirtyRect) {
  VSTGUI::ConcatClip cc(*context, dirtyRect);
  if (cc.isEmpty())
    return;
  context->setClipRect(dirtyRect);
  VSTGUI::CDrawContext::Transform transform(
      *context, VSTGUI::CGraphicsTransform().translate(getViewSize().left,
                                                       getViewSize().top));

  Oscillator o;
  auto buffer_size = size_t(getWidth() * 2);
  for (int gen = 0; gen < generators_.size(); gen++) {
    auto &gp = kPatch->generators_[gen];
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
    CArray fft_buffer(buffer_size);
    o.Perform(buffer_size, 44000, begin(fft_buffer), 440, a.data(), c.data(),
              m.data(), r.data(), s.data(), k.data());
    fft_buffer = fft_buffer / 100;
    HammingWindow(fft_buffer);
    FFT(fft_buffer);

    auto path = VSTGUI::owned(context->createGraphicsPath());
    if (path == nullptr)
      return;
    auto mid = getHeight() / 2;
    path->beginSubpath(
        VSTGUI::CPoint(0, mid + (buffer[0].real() * getHeight())));

    for (int j = 0; j < fft_buffer.size() / 2; j++)
      path->addLine(
          VSTGUI::CPoint(double(j), (1 - std::abs(fft_buffer[j])) * getHeight()));
    context->setFillColor(colours_[gen]);
    context->setFrameColor(colours_[gen]);
    context->setLineStyle(VSTGUI::kLineSolid);
    context->setLineWidth(2);
    context->setDrawMode(VSTGUI::kAntiAliasing | VSTGUI::kNonIntegralMode);
    context->drawGraphicsPath(path, VSTGUI::CDrawContext::kPathStroked);
  }
  setDirty(false);
}

bool SpectrumView::drawFocusOnTop() { return false; }

bool SpectrumView::getFocusPath(VSTGUI::CGraphicsPath &outPath) {
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

void SpectrumView::SetGenerators(const std::vector<int> &generators) {
  generators_ = generators;
  setDirty(true);
}

} // namespace ui
} // namespace sidebands
