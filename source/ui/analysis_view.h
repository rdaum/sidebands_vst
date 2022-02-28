#pragma once

#include <vstgui/vstgui.h>

namespace sidebands {
namespace ui {

class WaveformView;
class AnalysisView : public VSTGUI::CRowColumnView {
public:
  explicit AnalysisView(const VSTGUI::CRect &size);
private:
  WaveformView *waveform_view_;
};

}  // namespace ui
}  // namespace sidebands
