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

void ConvertToCArray(const OscBuffer &scalar, CArray &carray) {
  for (int i = 0; i < scalar.size(); i++) {
    carray[i] = scalar[i];
  }
}

// Cooley–Tukey FFT (in-place, divide-and-conquer)
// Higher memory requirements and redundancy although more intuitive
void FFT(CArray &x) {
  // DFT
  unsigned int N = x.size(), k = N, n;
  double thetaT = 3.14159265358979323846264338328L / N;
  Complex phiT = Complex(cos(thetaT), -sin(thetaT)), T;
  while (k > 1) {
    n = k;
    k >>= 1;
    phiT = phiT * phiT;
    T = 1.0L;
    for (unsigned int l = 0; l < k; l++) {
      for (unsigned int a = l; a < N; a += n) {
        unsigned int b = a + k;
        Complex t = x[a] - x[b];
        x[a] += x[b];
        x[b] = t * T;
      }
      T *= phiT;
    }
  }
  // Decimate
  unsigned int m = (unsigned int)log2(N);
  for (unsigned int a = 0; a < N; a++) {
    unsigned int b = a;
    // Reverse bits
    b = (((b & 0xaaaaaaaa) >> 1) | ((b & 0x55555555) << 1));
    b = (((b & 0xcccccccc) >> 2) | ((b & 0x33333333) << 2));
    b = (((b & 0xf0f0f0f0) >> 4) | ((b & 0x0f0f0f0f) << 4));
    b = (((b & 0xff00ff00) >> 8) | ((b & 0x00ff00ff) << 8));
    b = ((b >> 16) | (b << 16)) >> (32 - m);
    if (b > a) {
      Complex t = x[a];
      x[a] = x[b];
      x[b] = t;
    }
  }
  //// Normalize (This section make it not working correctly)
  // Complex f = 1.0 / sqrt(N);
  // for (unsigned int i = 0; i < N; i++)
  //	x[i] *= f;
}

void HanningWindow(CArray &buf) {
  auto samples_minus_1 = buf.size() - 1;
  for (int i = 0; i < buf.size(); i++) {
    auto multiplier =
        0.5 * (1 - cos(2 * std::numbers::pi * i / samples_minus_1));
    buf[i] = multiplier * buf[i];
  }
}

void HammingWindow(CArray &buf) {
  double samples_minus_1 = buf.size() - 1;
  for (int i = 0; i < buf.size(); i++) {
    auto multiplier =
        0.54 -
        (0.46 * std::cos(2 * std::numbers::pi * (double(i) / samples_minus_1)));
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
  auto buffer_size = size_t(1024);
  for (int gen = 0; gen < generators_.size(); gen++) {
    auto &gp = kPatch->generators_[gen];
    if (!gp->on())
      continue;
    OscBuffer osc_buffer(buffer_size);

    OscParam a(gp->a(), buffer_size);
    OscParam c(gp->c(), buffer_size);
    OscParam m(gp->m(), buffer_size);
    OscParam r(gp->r(), buffer_size);
    OscParam s(gp->s(), buffer_size);
    OscParam k(gp->k(), buffer_size);
    OscParam freq(2048, buffer_size);
    o.Perform(65536, osc_buffer, freq, c, m, r, s, k);

    CArray fft_buffer(osc_buffer.size());
    ConvertToCArray(osc_buffer, fft_buffer);
    HanningWindow(fft_buffer);
    FFT(fft_buffer);

    std::valarray<double> db_bins(buffer_size);
    for (int i = 0; i <= ((buffer_size)-1); i++) {
      db_bins[i] = std::abs(fft_buffer[i].real());
    }
    // Get the bin for the carrier, and scale things based on that.
    double max_bin = db_bins[65536/2048];
    auto scale_ratio = getHeight() / max_bin;
    auto path = VSTGUI::owned(context->createGraphicsPath());
    if (path == nullptr)
      return;
    path->beginSubpath(VSTGUI::CPoint(-1, db_bins[0] * scale_ratio));

    // Scale X axis from the first non-zero point until the nyquist point.
    VDArray indices(getWidth());
    linspace(indices, 0, db_bins.size() / 2, indices.size());
    for (int j = 0; j < getWidth(); j++) {
      path->addLine(VSTGUI::CPoint(
          double(j), getHeight() - db_bins[int(indices[j])] * scale_ratio));
    }
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
