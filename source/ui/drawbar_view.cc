#include "drawbar_view.h"

#include <glog/logging.h>
#include <pluginterfaces/vst/vsttypes.h>

#include "constants.h"
#include "sidebands_controller.h"
#include "ui/parameter_editor_view.h"

using Steinberg::Vst::ParamID;

namespace sidebands {
namespace ui {

namespace {
constexpr int kDrawbarWidth = 33;

VSTGUI::CResourceDescription kSelect("select.png");
VSTGUI::CResourceDescription kOnOff("on-off.png");
VSTGUI::CResourceDescription kToggleSwitch("toggle_switch.png");

}  // namespace

DrawbarView::DrawbarView(const VSTGUI::CRect &size,
                         SidebandsController *edit_controller)
    : PatchParameterView(edit_controller),
      VSTGUI::CRowColumnView(
          size, VSTGUI::CRowColumnView::kColumnStyle,
          VSTGUI::CRowColumnView::LayoutStyle::kCenterEqualy) {
  setBackgroundColor(kBgGrey);

  auto *label_column = new VSTGUI::CRowColumnView(
      VSTGUI::CRect(0, 0, 40, getHeight()), VSTGUI::CRowColumnView::kRowStyle,
      VSTGUI::CRowColumnView::LayoutStyle::kLeftTopEqualy, 2);
  auto *on_off_label = new VSTGUI::CView(VSTGUI::CRect(0, 0, 36, 15));
  on_off_label->setBackground(new VSTGUI::CBitmap(kOnOff));

  label_column->addView(on_off_label);
  label_column->addView(new VSTGUI::CView(VSTGUI::CRect(0, 0, 36, 275)));

  auto *select_label = new VSTGUI::CView(VSTGUI::CRect(0, 0, 36, 15));
  select_label->setBackground(new VSTGUI::CBitmap(kSelect));
  label_column->addView(select_label);

  addView(label_column);

  int selected_generator = SelectedGenerator(edit_controller);
  for (int drawbar_num = 0; drawbar_num < kNumGenerators; drawbar_num++) {
    auto *column = new VSTGUI::CRowColumnView(
        VSTGUI::CRect(0, 0, kDrawbarWidth, getHeight()),
        VSTGUI::CRowColumnView::kRowStyle,
        VSTGUI::CRowColumnView::LayoutStyle::kLeftTopEqualy, 2);
    column->setBackgroundColor(kBgGrey);
    toggle_buttons_[drawbar_num] = new VSTGUI::COnOffButton(
        VSTGUI::CRect(0, 0, column->getWidth(), 15), this,
        TagFor(drawbar_num, TAG_GENERATOR_TOGGLE, TARGET_NA),
        new VSTGUI::CBitmap(kToggleSwitch));
    toggle_buttons_[drawbar_num]->setValue(edit_controller->getParamNormalized(
        TagFor(drawbar_num, TAG_GENERATOR_TOGGLE, TARGET_NA)));
    column->addView(toggle_buttons_[drawbar_num]);
    column->addView(new ParameterEditorView(
        VSTGUI::CRect{0, 0, 40, getHeight() - 30},
        FindRangedParameter(edit_controller, drawbar_num, TAG_OSC, TARGET_A),
        this));

    select_buttons_[drawbar_num] = new VSTGUI::COnOffButton(
        VSTGUI::CRect(0, 0, column->getWidth(), 15), this,
        TagFor(drawbar_num, TAG_GENERATOR_SELECT, TARGET_NA),
        new VSTGUI::CBitmap(kToggleSwitch));
    column->addView(select_buttons_[drawbar_num]);

    select_buttons_[drawbar_num]->setValue(drawbar_num == selected_generator);

    addView(column);
  }
}

void DrawbarView::valueChanged(VSTGUI::CControl *control) {
  ParamID tag = control->getTag();
  ParamTag param = ParamFor(tag);
  if (param == TAG_GENERATOR_SELECT) {
    // Turn off other select buttons.
    for (auto *button : select_buttons_) {
      if (button != control) {
        button->setValue(0.0);
      }
    }
    setDirty(true);

    SelectGenerator(edit_controller(), GeneratorFor(tag));
    edit_controller()->selectUnit(
        MakeUnitID(UNIT_GENERATOR, GeneratorFor(tag)));
    return;
  }
  edit_controller()->UpdateParameterNormalized(tag,
                                               control->getValueNormalized());
}

void DrawbarView::update(Steinberg::FUnknown *unknown,
                         Steinberg::int32 message) {
  if (message != IDependent::kChanged) return;
  Steinberg::Vst::Parameter *changed_param;
  if (unknown->queryInterface(Steinberg::Vst::Parameter::iid,
                              (void **)&changed_param) != Steinberg::kResultOk)
    return;

  LOG(INFO) << "Update: " << TagStr(changed_param->getInfo().id);
  if (ParamFor(changed_param->getInfo().id) == TAG_GENERATOR_TOGGLE) {
    auto gen = GeneratorFor(changed_param->getInfo().id);
    toggle_buttons_[gen]->setValue(changed_param->getNormalized());
  }
}

}  // namespace ui
}  // namespace sidebands