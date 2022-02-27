#pragma once

#include <cstdint>
#include <glog/logging.h>
#include <public.sdk/source/vst/vsteditcontroller.h>
#include <public.sdk/source/vst/vstparameters.h>
#include <string>

namespace sidebands {

enum UnitTag {
  UNIT_NONE,
  UNIT_GENERATOR,
};

std::pair<UnitTag, uint16_t> ParseUnitID(Steinberg::Vst::UnitID unit_id);
Steinberg::Vst::UnitID MakeUnitID(UnitTag unit_tag, uint16_t unit_id);

enum ParamTag {
  TAG_GENERATOR_SELECT,
  TAG_GENERATOR_TOGGLE,
  TAG_OSC,
  TAG_ENV_A,
  TAG_ENV_AL,
  TAG_ENV_D,
  TAG_ENV_S,
  TAG_ENV_R,
  TAG_LFO_FREQ,
  TAG_LFO_AMP,
  TAG_LFO_TYPE,
  TAG_SELECTED_GENERATOR, // valid only with "0" for generator
  TAG_MOD_TYPE,
  TAG_NUM_TAGS
};

enum TargetTag {
  TARGET_NA,
  TARGET_C,
  TARGET_A,
  TARGET_M,
  TARGET_K,
  TARGET_R,
  TARGET_S,
  NUM_TARGETS
};

constexpr const char *kTargetNames[]{"NONE", "C", "A", "M", "K", "R", "S"};

constexpr const TargetTag kModulationTargets[]{
    TARGET_C, TARGET_A, TARGET_M, TARGET_K, TARGET_R, TARGET_S,
};

// Composed of both param and subparam, key for patch param lookup
struct ParamKey {
  ParamTag p_tag;
  TargetTag s_tag;

  bool operator==(const ParamKey &k) const {
    return k.p_tag == p_tag && k.s_tag == s_tag;
  }

  struct Hash {
    size_t operator()(const ParamKey &k) const {
      return std::hash<ParamTag>()(k.p_tag) ^ std::hash<TargetTag>()(k.s_tag);
    }
  };
};

uint8_t GeneratorFor(Steinberg::Vst::ParamID tag);
TargetTag TargetFor(Steinberg::Vst::ParamID tag);
ParamTag ParamFor(Steinberg::Vst::ParamID tag);

ParamKey ParamKeyFor(Steinberg::Vst::ParamID tag);
std::string TagStr(Steinberg::Vst::ParamID tag);
Steinberg::Vst::ParamID TagFor(uint8_t generator, ParamTag param,
                               TargetTag target);

Steinberg::Vst::RangeParameter *
FindRangedParameter(Steinberg::Vst::EditController *edit_controller,
                    uint16_t generator, const ParamTag &param,
                    const TargetTag &sp);

void SelectGenerator(Steinberg::Vst::IEditController *edit_controller,
                     int generator_number);
int SelectedGenerator(Steinberg::Vst::IEditController *edit_controller);

} // namespace sidebands