#pragma once

#include <vstgui/vstgui.h>

namespace sidebands {

static VSTGUI::CResourceDescription kSliderBackground("slider_rail.png");
static VSTGUI::CResourceDescription kSliderHandle("slider_handle.png");
static VSTGUI::CResourceDescription kSwitch("switches.png");
static VSTGUI::CResourceDescription kKnob("knob2.png");
static VSTGUI::CResourceDescription kSelect("select.png");
static VSTGUI::CResourceDescription kOnOff("on-off.png");
static VSTGUI::CResourceDescription kToggleSwitch("toggle_switch.png");
static VSTGUI::CResourceDescription kMetallicKnobBase("metallic_knob_base.png");
static VSTGUI::CResourceDescription kMetallicKnobHandle("metallic_knob_handle.png");

constexpr int kSliderWidth = 24;
constexpr int kSliderHeight = 189;
constexpr int kSliderMaxPos = 189;
constexpr int kNumericEditHeight = 15;
constexpr int kColumnLabelHeight = 15;

constexpr int kModRowHeight = kSliderHeight + kNumericEditHeight + kColumnLabelHeight;

constexpr int kDrawbarWidth = 24;

constexpr int kToggleButtonWidth = 24;
constexpr int kToggleButtonHeight = 15;

constexpr int kKnobWidth = 20;
constexpr int kKnobHeight = 20;
constexpr int kKnobSubpixmaps = 80;

}  // nemspace sidebands