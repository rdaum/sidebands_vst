#pragma once

#include <vstgui/vstgui.h>

namespace sidebands {
class SidebandsController;
namespace ui {

class WaveformView;
class SpectrumView;
class AnalysisView : public VSTGUI::CRowColumnView {
 public:
  AnalysisView(const VSTGUI::CRect &size, SidebandsController *sidebands_controller);
};

}  // namespace ui
}  // namespace sidebands
