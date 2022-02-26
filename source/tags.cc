#include "tags.h"

#include <cassert>

#include "constants.h"

namespace sidebands {

std::pair<UnitTag, uint16_t> ParseUnitID(Steinberg::Vst::UnitID unit_id) {
  return {static_cast<UnitTag>(unit_id >> 16), unit_id & 0x0000ffff};
}

Steinberg::Vst::UnitID MakeUnitID(UnitTag unit_tag, uint16_t unit_id) {
  return unit_tag << 16 | unit_id;
}

constexpr const char *kParamNames[]{
    "SELECT", "TOGGLE", "OSC", "ENV_A",         "ENV_AL",  "ENV_D",
    "ENV_S",  "ENV_R",  "LFO", "SELECTED_GEN#", "MOD_TYPE"};

std::string TagStr(Steinberg::Vst::ParamID tag) {
  std::string gen = std::to_string(GeneratorFor(tag));
  std::string param = kParamNames[ParamFor(tag)];
  std::string result = "#" + gen + "/" + param;
  TargetTag sp(TargetFor(tag));
  if (sp != TARGET_NA)
    result = result + "/" + kTargetNames[sp];
  return result;
}

uint8_t GeneratorFor(Steinberg::Vst::ParamID tag) {
  uint8_t gen = (tag >> 24);
  return gen;
}

ParamTag ParamFor(Steinberg::Vst::ParamID tag) {
  return static_cast<ParamTag>(tag & 0x000000ff);
}

TargetTag TargetFor(Steinberg::Vst::ParamID tag) {
  return static_cast<TargetTag>((tag & 0x00ffff00) >> 8);
}

Steinberg::Vst::ParamID TagFor(uint8_t generator, ParamTag param,
                               TargetTag target) {
  Steinberg::Vst::ParamID pid = (generator << 24 | target << 8 | param);
  return pid;
}

ParamKey ParamKeyFor(Steinberg::Vst::ParamID tag) {
  return {ParamFor(tag), TargetFor(tag)};
}

Steinberg::Vst::RangeParameter *
FindRangedParameter(Steinberg::Vst::EditController *edit_controller,
                    uint16_t generator, const ParamTag &param,
                    const TargetTag &sp) {
  Steinberg::Vst::ParamID tag = TagFor(generator, param, sp);
  auto *param_obj = edit_controller->getParameterObject(tag);
  if (!param_obj)
    return nullptr;
  Steinberg::Vst::RangeParameter *ranged_parameter;
  auto result = param_obj->queryInterface(Steinberg::Vst::RangeParameter::iid,
                                          (void **)&ranged_parameter);
  CHECK_EQ(result, Steinberg::kResultOk);
  return ranged_parameter;
}

void SelectGenerator(Steinberg::Vst::IEditController *edit_controller,
                     int generator_number) {
  edit_controller->setParamNormalized(
      TagFor(0, TAG_SELECTED_GENERATOR, TARGET_NA),
      generator_number / (float)kNumGenerators);
}

int SelectedGenerator(Steinberg::Vst::IEditController *edit_controller) {
  return edit_controller->getParamNormalized(
             TagFor(0, TAG_SELECTED_GENERATOR, TARGET_NA)) *
         kNumGenerators;
}
} // namespace sidebands
