#include "ui/analysis_view.h"

#include "constants.h"
#include "ui/waveform_view.h"
#include "ui/spectrum_view.h"

namespace sidebands {
namespace ui {

using VSTGUI::CColor;

AnalysisView::AnalysisView(const VSTGUI::CRect &size)
    : VSTGUI::CRowColumnView(size) {
  setBackgroundColor(kBgGrey);

  std::vector<CColor> colours{
      {0x1a, 0x13, 0x34}, {0x26, 0x29, 0x4a}, {0x01, 0x54, 0x5a},
      {0x01, 0x73, 0x51}, {0x03, 0xc3, 0x83}, {0xaa, 0xd9, 0x62},
      {0xfb, 0xbf, 0x45}, {0xef, 0x6a, 0x32}, {0xed, 0x03, 0x45},
      {0xa1, 0x2a, 0x5e}, {0x71, 0x01, 0x62}, {0x02, 0x2c, 0x7d},
      {0x1a, 0x13, 0x34}, {0x26, 0x29, 0x4a}, {0x01, 0x54, 0x5a},
      {0x1a, 0x13, 0x34}, {0x26, 0x29, 0x4a}};
  std::vector<int> generators(kNumGenerators);
  for (uint8_t i = 0; i < kNumGenerators; i++) {
    generators[i] = i;
  }
  addView(new WaveformView(VSTGUI::CRect(0, 0, getWidth(), getHeight()/2), colours, generators));
  addView(new SpectrumView(VSTGUI::CRect(0, 0, getWidth(), getHeight()/2), colours, generators));
}

} // namespace ui
} // namespace sidebands