#include "controller/ui/generator_editor_view.h"

#include <absl/strings/str_format.h>
#include <glog/logging.h>
#include <pluginterfaces/vst/vsttypes.h>
#include <public.sdk/source/vst/vstparameters.h>

#include "constants.h"
#include "controller/sidebands_controller.h"
#include "controller/ui/mod_target_view.h"
#include "controller/ui/parameter_editor_view.h"
#include "controller/ui/spectrum_view.h"
#include "controller/ui/waveform_view.h"

using Steinberg::Vst::Parameter;
using Steinberg::Vst::ParameterInfo;
using Steinberg::Vst::ParamID;

namespace sidebands {
namespace ui {

namespace {

constexpr int kModRowHeight = 285;

} // namespace

GeneratorEditorView::GeneratorEditorView(const VSTGUI::CRect &size,
                                         SidebandsController *edit_controller)
    : VSTGUI::CScrollView(
          size, VSTGUI::CRect(0, 0, size.getWidth(), kModRowHeight * 4),
          CScrollView::kVerticalScrollbar),
      edit_controller_(edit_controller) {
  auto *sel_gen_param = edit_controller->getParameterObject(
      TagFor(0, TAG_SELECTED_GENERATOR, TARGET_NA));
  sel_gen_param->addDependent(this);
  setBackgroundColor(kBgGrey);

  int selected_generator = edit_controller->SelectedGenerator();

  auto *modulator_rows = new VSTGUI::CRowColumnView(
      VSTGUI::CRect(0, 0, getWidth() - 40, kModRowHeight * 4),
      VSTGUI::CRowColumnView::kRowStyle,
      VSTGUI::CRowColumnView::LayoutStyle::kLeftTopEqualy, 2);
  modulator_rows->setBackgroundColor(kBgGrey);

  selected_label_ = new VSTGUI::CTextLabel(
      VSTGUI::CRect(0, 0, getWidth(), 20),
      absl::StrFormat("#%d", selected_generator).c_str());

  VSTGUI::CRect column_size{0, 0, 40, modulator_rows->getHeight()};
  c_slider_ =
      new ParameterEditorView(column_size,
                              edit_controller->FindRangedParameter(
                                  selected_generator, TAG_OSC, TARGET_C),
                              this, "C");

  m_slider_ =
      new ParameterEditorView(column_size,
                              edit_controller->FindRangedParameter(
                                  selected_generator, TAG_OSC, TARGET_M),
                              this, "M");
  k_slider_ =
      new ParameterEditorView(column_size,
                              edit_controller->FindRangedParameter(
                                  selected_generator, TAG_OSC, TARGET_K),
                              this, "K");
  r_slider_ =
      new ParameterEditorView(column_size,
                              edit_controller->FindRangedParameter(
                                  selected_generator, TAG_OSC, TARGET_R),
                              this, "R");
  s_slider_ =
      new ParameterEditorView(column_size,
                              edit_controller->FindRangedParameter(
                                  selected_generator, TAG_OSC, TARGET_S),
                              this, "S");

  a_target_view_ = new ModulatorTargetView(
      VSTGUI::CRect(0, 0, modulator_rows->getWidth() - c_slider_->getWidth(),
                    kModRowHeight),
      edit_controller, TARGET_A);

  k_target_view_ = new ModulatorTargetView(
      VSTGUI::CRect(0, 0,
                    modulator_rows->getWidth() - m_slider_->getWidth() -
                        k_slider_->getWidth(),
                    kModRowHeight),
      edit_controller, TARGET_K);

  auto *analysis_area = new VSTGUI::CRowColumnView(
      VSTGUI::CRect(0, 0,
                    getWidth() - k_slider_->getWidth() - c_slider_->getWidth() -
                        m_slider_->getWidth(),
                    kModRowHeight),
      VSTGUI::CRowColumnView::kRowStyle);

  waveform_view_ = new WaveformView(
      VSTGUI::CRect(0, 0, analysis_area->getWidth(), kModRowHeight / 2),
      {VSTGUI::CColor(0, 0, 100)}, {selected_generator}, edit_controller);

  spectrum_view_ = new SpectrumView(
      VSTGUI::CRect(0, 0, analysis_area->getWidth(), kModRowHeight / 2),
      {VSTGUI::CColor(0, 0, 100)}, {selected_generator}, edit_controller);
  analysis_area->addView(waveform_view_);
  analysis_area->addView(spectrum_view_);

  modulator_rows->addView(selected_label_);
  auto *osc_columns =
      new VSTGUI::CRowColumnView(VSTGUI::CRect(0, 0, modulator_rows->getWidth(),
                                               analysis_area->getHeight()),
                                 VSTGUI::CRowColumnView::kColumnStyle);

  osc_columns->setBackgroundColor(kBgGrey);
  modulator_rows->addView(osc_columns);

  osc_columns->addView(c_slider_);
  osc_columns->addView(m_slider_);
  osc_columns->addView(k_slider_);
  osc_columns->addView(r_slider_);
  osc_columns->addView(s_slider_);
  osc_columns->addView(analysis_area);

  auto *a_columns =
      new VSTGUI::CRowColumnView(VSTGUI::CRect(0, 0, modulator_rows->getWidth(),
                                               a_target_view_->getHeight()),
                                 VSTGUI::CRowColumnView::kColumnStyle);

  a_columns->addView(a_target_view_);

  modulator_rows->addView(a_columns);

  auto *k_columns =
      new VSTGUI::CRowColumnView(VSTGUI::CRect(0, 0, modulator_rows->getWidth(),
                                               k_target_view_->getHeight()),
                                 VSTGUI::CRowColumnView::kColumnStyle);
  k_columns->addView(k_target_view_);
  modulator_rows->addView(k_columns);
  addView(modulator_rows);
}

GeneratorEditorView::~GeneratorEditorView() {
  auto *selected_gen_param = edit_controller_->FindRangedParameter(
      0, TAG_SELECTED_GENERATOR, TARGET_NA);
  selected_gen_param->removeDependent(this);
}

void GeneratorEditorView::valueChanged(VSTGUI::CControl *control) {
  // the tag on the scrollbar is the same as our "toggle" tag! don't handle it.
  if (control == getVerticalScrollbar() ||
      control == getHorizontalScrollbar()) {
    CScrollView::valueChanged(control);
    return;
  }
  ParamID tag = control->getTag();
  edit_controller_->UpdateParameterNormalized(tag,
                                              control->getValueNormalized());

  setDirty(true);
}

void GeneratorEditorView::update(Steinberg::FUnknown *changedUnknown,
                                 Steinberg::int32 message) {
  if (message == IDependent::kChanged) {
    Steinberg::Vst::RangeParameter *changed_param;
    auto query_result = changedUnknown->queryInterface(
        Steinberg::Vst::RangeParameter::iid, (void **)&changed_param);
    CHECK(query_result == Steinberg::kResultOk);
    if (changed_param->getInfo().id ==
        TagFor(0, TAG_SELECTED_GENERATOR, TARGET_NA)) {
      int32_t new_generator =
          int32_t(changed_param->getNormalized() * kNumGenerators);

      k_target_view_->SwitchGenerator(new_generator);
      a_target_view_->SwitchGenerator(new_generator);
      waveform_view_->SetGenerators({new_generator});

      c_slider_->UpdateControlParameters(edit_controller_,
                                         edit_controller_->FindRangedParameter(
                                             new_generator, TAG_OSC, TARGET_C));
      m_slider_->UpdateControlParameters(edit_controller_,
                                         edit_controller_->FindRangedParameter(
                                             new_generator, TAG_OSC, TARGET_M));
      k_slider_->UpdateControlParameters(edit_controller_,
                                         edit_controller_->FindRangedParameter(
                                             new_generator, TAG_OSC, TARGET_K));

      r_slider_->UpdateControlParameters(edit_controller_,
                                         edit_controller_->FindRangedParameter(
                                             new_generator, TAG_OSC, TARGET_R));

      s_slider_->UpdateControlParameters(edit_controller_,
                                         edit_controller_->FindRangedParameter(
                                             new_generator, TAG_OSC, TARGET_S));
      selected_label_->setText(absl::StrFormat("#%d", new_generator).c_str());
      setDirty(true);

      return;
    }
  }
}

} // namespace ui
} // namespace sidebands