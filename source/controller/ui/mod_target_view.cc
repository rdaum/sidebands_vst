#include "controller/ui/mod_target_view.h"

#include <absl/strings/str_format.h>

#include "controller/sidebands_controller.h"
#include "controller/ui/envelope_editor_view.h"
#include "controller/ui/gui_constants.h"
#include "controller/ui/lfo_editor_view.h"

namespace sidebands {
namespace ui {

ModulatorTargetView::ModulatorTargetView(const VSTGUI::CRect &size,
                                         SidebandsController *edit_controller,
                                         TargetTag target)
    : VSTGUI::CRowColumnView(
          size, VSTGUI::CRowColumnView::kRowStyle,
          VSTGUI::CRowColumnView::LayoutStyle::kLeftTopEqualy, 2),
      ModulatorEditorView(edit_controller, target) {
  const int selected_generator = edit_controller->SelectedGenerator();

  setBackgroundColor(VSTGUI::kTransparentCColor);

  auto *title_row = new VSTGUI::CRowColumnView(
      VSTGUI::CRect(0, 0, getWidth(), kTitleBarHeight),
      VSTGUI::CRowColumnView::kColumnStyle,
      VSTGUI::CRowColumnView::LayoutStyle::kLeftTopEqualy, 2);

  std::string title = absl::StrFormat("%s modulation", kTargetLongNames[target]);
  title_row->addView(new VSTGUI::CTextLabel(
      VSTGUI::CRect(0, 0, 200, kTitleBarHeight), title.c_str()));

  if (target != TARGET_A) {
    mod_source_selector_ = new VSTGUI::COptionMenu(
        VSTGUI::CRect(0, 0, 100, kTitleBarHeight), this,
        TagFor(selected_generator, TAG_MOD_TYPE, target));
    mod_source_selector_->addEntry(new VSTGUI::CMenuItem("Constant"));
    mod_source_selector_->addEntry(new VSTGUI::CMenuItem("Envelope"));
    mod_source_selector_->addEntry(new VSTGUI::CMenuItem("LFO"));
    title_row->addView(mod_source_selector_);
  }

  addView(title_row);

  envelope_editor_view_ = new EnvelopeEditorView(
      VSTGUI::CRect(0, 0, getWidth(), size.getHeight() - kTitleBarHeight - kModRowPadding),
      edit_controller, target);
  lfo_editor_view_ = new LFOEditorView(
      VSTGUI::CRect(0, 0, getWidth(), size.getHeight() - kTitleBarHeight - kModRowPadding),
      edit_controller, target);

  addView(envelope_editor_view_);
  addView(lfo_editor_view_);
  SwitchViewVisibility();
}

void ModulatorTargetView::valueChanged(VSTGUI::CControl *control) {
  Steinberg::Vst::ParamID tag = control->getTag();
  edit_controller()->UpdateParameterNormalized(tag,
                                               control->getValueNormalized());

  if (control == mod_source_selector_) {
    SwitchViewVisibility();
  }
}

void ModulatorTargetView::SwitchGenerator(int new_generator) {
  SwitchViewVisibility();
  envelope_editor_view_->SwitchGenerator(new_generator);
  lfo_editor_view_->SwitchGenerator(new_generator);
  if (mod_source_selector_) {
    Steinberg::Vst::ParamID old_tag = mod_source_selector_->getTag();
    auto *old_param_obj = edit_controller()->getParameterObject(old_tag);
    if (!old_param_obj) {
      LOG(ERROR) << "Missing parameter for tag: " << TagStr(old_tag);
      return;
    }

    Steinberg::Vst::ParamID new_tag =
        TagFor(new_generator, TAG_MOD_TYPE, target());
    auto *new_param_obj = edit_controller()->getParameterObject(new_tag);
    mod_source_selector_->setTag(new_tag);
    mod_source_selector_->setValueNormalized(new_param_obj->getNormalized());
    mod_source_selector_->setDirty(true);
  }
  setDirty(true);
}

void ModulatorTargetView::SwitchViewVisibility() {
  auto mod_type_v = edit_controller()->getParamNormalized(
      TagFor(edit_controller()->SelectedGenerator(), TAG_MOD_TYPE, target()));
  auto mod_type = kModTypes[int(mod_type_v * (kNumModTypes - 1))];

  if (mod_type != ModType::LFO) {
    lfo_editor_view_->setVisible(false);
    lfo_editor_view_->setViewSize(VSTGUI::CRect(0, 0, 0, 0));
  }

  if (mod_type != ModType::ADSR_ENVELOPE) {
    envelope_editor_view_->setVisible(false);
    envelope_editor_view_->setViewSize(VSTGUI::CRect(0, 0, 0, 0));
  }

  if (mod_type == ModType::LFO) {
    lfo_editor_view_->setVisible(true);
    lfo_editor_view_->setViewSize(getViewSize());
  }

  if (mod_type == ModType::ADSR_ENVELOPE) {
    envelope_editor_view_->setVisible(true);
    envelope_editor_view_->setViewSize(getViewSize());
  }

  layoutViews();
  setDirty(true);
}

void ModulatorTargetView::HighlightEnvelopeStage(off_t stage) {
  envelope_editor_view_->HighlightEnvelopeStage(stage);
}

} // namespace ui
} //  namespace sidebands
