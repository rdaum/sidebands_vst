#include "sidebands_controller.h"

#include <absl/strings/str_format.h>
#include <glog/logging.h>
#include <pluginterfaces/base/ustring.h>
#include <vstgui/plugin-bindings/vst3editor.h>
#include <vstgui/uidescription/uiattributes.h>
#include <vstgui/vstgui.h>

#include "base/source/fstreamer.h"
#include "globals.h"
#include "sidebands_cids.h"
#include "tags.h"
#include "controller/ui/analysis_view.h"
#include "controller/ui/drawbar_view.h"
#include "controller/ui/generator_editor_view.h"
#include "controller/patch_controller.h"

using namespace Steinberg;

namespace sidebands {

// static
Steinberg::FUnknown *SidebandsController::Instantiate(void *) {
  google::InitGoogleLogging("sidebands");
  FLAGS_stderrthreshold = 0;

  return (Steinberg::Vst::IEditController *)new SidebandsController;
}

tresult PLUGIN_API SidebandsController::initialize(FUnknown *context) {
  tresult result = EditControllerEx1::initialize(context);
  if (result != kResultOk) {
    return result;
  }

  LOG(INFO) << "Creating units...";
  for (int g = 0; g < kNumGenerators; g++) {
    Steinberg::Vst::UnitInfo unitInfo;
    unitInfo.id = MakeUnitID(UNIT_GENERATOR, g);
    unitInfo.parentUnitId = Steinberg::Vst::kRootUnitId;
    std::string full_name = absl::StrFormat("Generator Unit %d", g);
    Steinberg::UString(unitInfo.name, USTRINGSIZE(unitInfo.name))
        .assign(USTRING(full_name.c_str()));
    unitInfo.programListId = Steinberg::Vst::kNoProgramListId;
    Steinberg::Vst::Unit *unit = new Steinberg::Vst::Unit(unitInfo);
    addUnit(unit);
  }

  LOG(INFO) << "Declaring units and parameters";
  PatchController::AppendParameters(&parameters);

  // Universal parameters, not generator specific.
  std::string sel_gen_param_name = "Selected Generator Number";
  auto parameter_info =
      Vst::ParameterInfo{.id = TagFor(0, TAG_SELECTED_GENERATOR, TARGET_NA),
                         .defaultNormalizedValue = 0,
                         .unitId = Vst::kRootUnitId};
  Steinberg::UString(parameter_info.title, USTRINGSIZE(parameter_info.title))
      .assign(USTRING(sel_gen_param_name.c_str()));
  parameters.addParameter(parameter_info);

  LOG(INFO) << "Initialization complete";

  return result;
}

tresult PLUGIN_API SidebandsController::terminate() {
  LOG(INFO) << "Terminating controller";

  //---do not forget to call parent ------
  return EditControllerEx1::terminate();
}

tresult PLUGIN_API SidebandsController::setComponentState(IBStream *state) {
  // Here you get the state of the component (Processor part)
  if (!state) return kResultFalse;

  startGroupEdit();

  IBStreamer streamer(state, kLittleEndian);
  for (int i = 0; i < kNumGenerators; i++) {
    UpdateParameterNormalized(TagFor(i, TAG_OSC, TARGET_A), 0.25);

    UpdateParameterNormalized(TagFor(i, TAG_OSC, TARGET_C),
                              float(1 + i) / kNumGenerators);
    UpdateParameterNormalized(TagFor(i, TAG_MOD_TYPE, TARGET_A),
                              double(ModType::ENVELOPE) / (kNumModTypes - 1));
    UpdateParameterNormalized(TagFor(i, TAG_MOD_TYPE, TARGET_K),
                              double(ModType::ENVELOPE) / (kNumModTypes - 1));
    setParamNormalized(TagFor(i, TAG_OSC, TARGET_K), 0.5);
    setParamNormalized(TagFor(i, TAG_OSC, TARGET_M), 0.5);
    UpdateParameterNormalized(TagFor(i, TAG_OSC, TARGET_R), 1);
    UpdateParameterNormalized(TagFor(i, TAG_OSC, TARGET_S), 1);

    UpdateParameterNormalized(TagFor(i, TAG_ENV_A, TARGET_A), 0.05);
    UpdateParameterNormalized(TagFor(i, TAG_ENV_AL, TARGET_A), 1);
    UpdateParameterNormalized(TagFor(i, TAG_ENV_D, TARGET_A), 0.2);
    UpdateParameterNormalized(TagFor(i, TAG_ENV_S, TARGET_A), 0.25);
    UpdateParameterNormalized(TagFor(i, TAG_ENV_R, TARGET_A), 0.2);
    UpdateParameterNormalized(TagFor(i, TAG_ENV_VS, TARGET_A), 1.0);

    UpdateParameterNormalized(TagFor(i, TAG_ENV_A, TARGET_K), 0.00);
    UpdateParameterNormalized(TagFor(i, TAG_ENV_AL, TARGET_K), 1);
    UpdateParameterNormalized(TagFor(i, TAG_ENV_D, TARGET_K), 0.25);
    UpdateParameterNormalized(TagFor(i, TAG_ENV_S, TARGET_K), 0.25);
    UpdateParameterNormalized(TagFor(i, TAG_ENV_R, TARGET_K), 0.3);
    UpdateParameterNormalized(TagFor(i, TAG_ENV_VS, TARGET_K), 1);

    UpdateParameterNormalized(TagFor(i, TAG_LFO_FREQ, TARGET_K), 0.01);
    UpdateParameterNormalized(TagFor(i, TAG_LFO_AMP, TARGET_K), 0.5);
    UpdateParameterNormalized(TagFor(i, TAG_LFO_VS, TARGET_K), 1);
    UpdateParameterNormalized(TagFor(i, TAG_LFO_TYPE, TARGET_K), 0);
  }
  UpdateParameterNormalized(TagFor(0, TAG_GENERATOR_TOGGLE, TARGET_NA), 1);

  finishGroupEdit();

  setParamNormalized(TagFor(0, TAG_SELECTED_GENERATOR, TARGET_NA), 0);

  return kResultOk;
}

tresult PLUGIN_API SidebandsController::setState(IBStream *state) {
  // Here you get the state of the controller

  return kResultTrue;
}

tresult PLUGIN_API SidebandsController::getState(IBStream *state) {
  // Here you are asked to deliver the state of the controller (if needed)
  // Note: the real state of your plug-in is saved in the processor

  return kResultTrue;
}

IPlugView *PLUGIN_API SidebandsController::createView(FIDString name) {
  // Here the Host wants to open your editor (if you have one)
  if (FIDStringsEqual(name, Vst::ViewType::kEditor)) {
    // create your editor here and return a IPlugView ptr of it
    auto *view = new VSTGUI::VST3Editor(this, "view", "sidebands.uidesc");
    return view;
  }
  return nullptr;
}

tresult PLUGIN_API SidebandsController::setParamNormalized(
    Vst::ParamID tag, Vst::ParamValue value) {
  // called by host to update your parameters
  tresult result = EditControllerEx1::setParamNormalized(tag, value);
  CHECK_EQ(result, kResultTrue) << "Unable to set: " << TagStr(tag);
  if (analysis_view_) {
    analysis_view_->setDirty(true);
  }
  LOG(INFO) << "Set: " << TagStr(tag) << " to normalized: " << value;
  return result;
}

tresult PLUGIN_API SidebandsController::getParamStringByValue(
    Vst::ParamID tag, Vst::ParamValue valueNormalized, Vst::String128 string) {
  // called by host to get a string for given normalized value of a specific
  // parameter (without having to set the value!)
  return EditControllerEx1::getParamStringByValue(tag, valueNormalized, string);
}

tresult PLUGIN_API SidebandsController::getParamValueByString(
    Vst::ParamID tag, Vst::TChar *string, Vst::ParamValue &valueNormalized) {
  // called by host to get a normalized value from a string representation of a
  // specific parameter (without having to set the value!)
  return EditControllerEx1::getParamValueByString(tag, string, valueNormalized);
}

VSTGUI::CView *SidebandsController::createCustomView(
    VSTGUI::UTF8StringPtr name, const VSTGUI::UIAttributes &attributes,
    const VSTGUI::IUIDescription *description, VSTGUI::VST3Editor *editor) {
  const auto &view_name = VSTGUI::UTF8StringView(name);

  VSTGUI::CPoint origin;
  attributes.getPointAttribute("origin", origin);
  VSTGUI::CPoint size;
  attributes.getPointAttribute("size", size);
  if (view_name == "DrawbarEditor") {
    return new ui::DrawbarView(VSTGUI::CRect(origin, size), this);
  }
  if (view_name == "GeneratorEditor") {
    return new ui::GeneratorEditorView(VSTGUI::CRect(origin, size), this);
  }
  if (view_name == "AnalysisView") {
    analysis_view_ = new ui::AnalysisView(VSTGUI::CRect(origin, size), this);
    return analysis_view_;
  }

  return nullptr;
}

void SidebandsController::UpdateParameterNormalized(
    Steinberg::Vst::ParamID param_id, Steinberg::Vst::ParamValue value) {
  setParamNormalized(param_id, value);
  beginEdit(param_id);
  performEdit(param_id, value);
  endEdit(param_id);
}

Steinberg::Vst::RangeParameter *SidebandsController::FindRangedParameter(
    uint16_t generator, const ParamTag &param, const TargetTag &sp) {
  Steinberg::Vst::ParamID tag = TagFor(generator, param, sp);
  auto *param_obj = getParameterObject(tag);
  if (!param_obj)
    return nullptr;
  Steinberg::Vst::RangeParameter *ranged_parameter;
  auto result = param_obj->queryInterface(Steinberg::Vst::RangeParameter::iid,
                                          (void **)&ranged_parameter);
  CHECK_EQ(result, Steinberg::kResultOk);
  return ranged_parameter;
}

void SidebandsController::SelectGenerator(int generator_number) {
  UpdateParameterNormalized(TagFor(0, TAG_SELECTED_GENERATOR, TARGET_NA),
                            generator_number / (float)kNumGenerators);
}

int SidebandsController::SelectedGenerator() {
  return getParamNormalized(TagFor(0, TAG_SELECTED_GENERATOR, TARGET_NA)) *
         kNumGenerators;
}

Steinberg::Vst::ParamValue
SidebandsController::GetParamValue(Steinberg::Vst::ParamID param_id) {
  auto *param_obj = getParameterObject(param_id);
  if (!param_obj) return 0;
  Steinberg::Vst::RangeParameter *ranged_parameter;
  auto result = param_obj->queryInterface(Steinberg::Vst::RangeParameter::iid,
                                          (void **)&ranged_parameter);
  CHECK_EQ(result, Steinberg::kResultOk);
  return ranged_parameter->toPlain(ranged_parameter->getNormalized());
}

} // namespace sidebands