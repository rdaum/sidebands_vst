#include "ui/analysis_view.h"

#include "constants.h"
#include "ui/waveform_view.h"

namespace sidebands {
namespace ui {

using VSTGUI::CColor;

AnalysisView::AnalysisView(const VSTGUI::CRect &size)
 : VSTGUI::CRowColumnView(size) {
  setBackgroundColor(kBgGrey);

  std::vector<int> generators(kNumGenerators);
  std::vector<CColor> colours(kNumGenerators);
  uint8_t interval = 256 / kNumGenerators;
  for (uint8_t i = 0 ; i < kNumGenerators; i++) {
    generators[i] = i;
    colours[i] = CColor{0, 0,  uint8_t(100 + (i * interval))};
  }
  addView(new WaveformView(size, colours, generators));
}

}  // namespace ui
}  // namespace sidebands