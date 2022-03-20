#pragma once

namespace sidebands {

constexpr const char *
    kVerticalSliderLargeBackground("slider_rail_vertical_large.png");
constexpr const char *
    kVerticalSliderSmallBackground("slider_rail_vertical_small.png");
constexpr const char *
    kHorizontalSliderSmallBackground("slider_rail_horizontal_small.png");

constexpr const char *kSliderHandle("slider_handle.png");
constexpr const char *kSelect("select.png");
constexpr const char *kOnOff("on-off.png");
constexpr const char *kToggleSwitch("toggle_switch.png");
constexpr const char *kMetallicKnobBase("metallic_knob_base.png");
constexpr const char *kMetallicKnobHandle("metallic_knob_handle.png");

struct SliderDesc {
  double width, height, max_pos;
  const char *background;
  const char *handle;
};
constexpr SliderDesc kVerticalSliderTallDimensions{
    24, 189, 189, kVerticalSliderLargeBackground, kSliderHandle};
constexpr SliderDesc kVerticalSliderShortDimensions{
    24, 59, 59, kVerticalSliderSmallBackground, kSliderHandle};
constexpr SliderDesc kHorizontalSliderShortDimensions{
    59, 24, 59, kHorizontalSliderSmallBackground, kSliderHandle};

constexpr double kNumericEditWidth = 24;
constexpr double kNumericEditHeight = 15;
constexpr double kColumnLabelHeight = 15;
constexpr double kTitleBarHeight = 15;
constexpr double kModRowPadding = 30;

constexpr double kModRowHeight = kVerticalSliderShortDimensions.height +
                                 kNumericEditHeight + kColumnLabelHeight + kTitleBarHeight + kModRowPadding;

constexpr double kDrawbarWidth = 24;
constexpr double kToggleButtonWidth = 36;
constexpr double kToggleButtonHeight = 15;

} // namespace sidebands