#pragma once

#include <public.sdk/source/vst/vsteditcontroller.h>
#include <vstgui/vstgui.h>

#include "constants.h"
#include "synthesis/oscillator.h"
#include "tags.h"

namespace sidebands {
namespace ui {

// Plots the waveform of the oscillator output for a specific generator
class WaveformView : public VSTGUI::CView, public VSTGUI::IFocusDrawing {
 public:
  WaveformView(const VSTGUI::CRect &size,
               const std::vector<VSTGUI::CColor> colours,
               const std::vector<int> &generators);

  // CView overrides
  void draw(VSTGUI::CDrawContext *pContext) override;
  void drawRect(VSTGUI::CDrawContext *context,
                const VSTGUI::CRect &dirtyRect) override;

  // IFocusDrawing overrides
  bool drawFocusOnTop() override;
  bool getFocusPath(VSTGUI::CGraphicsPath &outPath) override;

  void SetGenerators(const std::vector<int> &generators);

 private:
  std::vector<VSTGUI::CColor> colours_;
  std::vector<int> generators_;
};

}  // namespace ui
}  // namespace sidebands