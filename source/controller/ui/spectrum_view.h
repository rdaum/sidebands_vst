#pragma once

#include <public.sdk/source/vst/vsteditcontroller.h>
#include <vstgui/vstgui.h>

#include "constants.h"
#include "processor/synthesis/oscillator.h"
#include "tags.h"

namespace sidebands {

class SidebandsController;
namespace ui {

// Plots the Spectrum of the oscillator output for a specific generator
class SpectrumView : public VSTGUI::CView, public VSTGUI::IFocusDrawing {
 public:
  SpectrumView(const VSTGUI::CRect &size,
               const std::vector<VSTGUI::CColor> colours,
               const std::vector<int> &generators,
               SidebandsController *sidebands_controller);

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
  SidebandsController *sidebands_controller_;
};

}  // namespace ui
}  // namespace sidebands