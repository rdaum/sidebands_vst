#include "ui/mod_target_view.h"

#include "synthesis/patch.h"
#include "ui/envelope_editor_view.h"
#include "ui/lfo_editor_view.h"

namespace sidebands {

ModulatorTargetView::ModulatorTargetView(
    const VSTGUI::CRect &size, Steinberg::Vst::EditController *edit_controller,
    TargetTag target)
    : VSTGUI::CRowColumnView(
          size, VSTGUI::CRowColumnView::kRowStyle,
          VSTGUI::CRowColumnView::LayoutStyle::kLeftTopEqualy, 2),
      ModulatorEditorView(edit_controller, target) {
  setBackgroundColor(kBgGrey);
  int selected_generator = SelectedGenerator(edit_controller);

  if (target != TARGET_A) {
    auto *modulation_selector_row = new VSTGUI::CRowColumnView(
        VSTGUI::CRect(0, 0, getWidth(), 15),
        VSTGUI::CRowColumnView::kColumnStyle,
        VSTGUI::CRowColumnView::LayoutStyle::kLeftTopEqualy, 2);
    modulation_selector_row->addView(
        new VSTGUI::CTextLabel(VSTGUI::CRect(0, 0, 100, 15), "Modulation"));
    mod_source_selector_ = new VSTGUI::COptionMenu(
        VSTGUI::CRect(0, 0, 100, 15), this,
        TagFor(selected_generator, TAG_MOD_TYPE, target));
    mod_source_selector_->addEntry(new VSTGUI::CMenuItem("Constant"));
    mod_source_selector_->addEntry(new VSTGUI::CMenuItem("Envelope"));
    mod_source_selector_->addEntry(new VSTGUI::CMenuItem("LFO"));
    modulation_selector_row->addView(mod_source_selector_);
    addView(modulation_selector_row);
  }

  envelope_editor_view_ = new EnvelopeEditorView(
      VSTGUI::CRect(0, 0, getWidth(), getHeight()), edit_controller, target);
  lfo_editor_view_ = new LFOEditorView(
      VSTGUI::CRect(0, 0, getWidth(), getHeight()), edit_controller, target);

  SwitchViewVisibility();
}

void ModulatorTargetView::valueChanged(VSTGUI::CControl *control) {
  ModulatorEditorView::valueChanged(control);
  if (control == mod_source_selector_) {
    SwitchViewVisibility();
  }
}

void ModulatorTargetView::SwitchGenerator(int new_generator) {
  envelope_editor_view_->SwitchGenerator(new_generator);
  lfo_editor_view_->SwitchGenerator(new_generator);
  SwitchViewVisibility();
  setDirty(true);
}

void ModulatorTargetView::SwitchViewVisibility() {
  auto mod_type_v = edit_controller()->getParamNormalized(
      TagFor(SelectedGenerator(edit_controller()), TAG_MOD_TYPE, target()));
  auto mod_type = GeneratorPatch::kModTypes[int(
      mod_type_v * (GeneratorPatch::kNumModTypes - 1))];

  if (lfo_editor_view_->getParentView() == this &&
      mod_type != GeneratorPatch::ModType::LFO)
    removeView(lfo_editor_view_);

  if (envelope_editor_view_->getParentView() == this &&
      mod_type != GeneratorPatch::ModType::ENVELOPE)
    removeView(envelope_editor_view_);

  if (mod_type == GeneratorPatch::ModType::ENVELOPE) {
    addView(envelope_editor_view_);
  } else if (mod_type == GeneratorPatch::ModType::LFO) {
    addView(lfo_editor_view_);
  }


  setDirty(true);
}

} //  namespace sidebands
