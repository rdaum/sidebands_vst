#pragma once

#include <glog/logging.h>
#include <public.sdk/source/vst/vstparameters.h>

#include <cstdint>
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
  TAG_ENV_HT,
  TAG_ENV_AR,
  TAG_ENV_AL,
  TAG_ENV_DR1,
  TAG_ENV_DL1,
  TAG_ENV_DR2,
  TAG_ENV_SL, // sustain
  TAG_ENV_RR1,
  TAG_ENV_RL1,
  TAG_ENV_RR2,
  TAG_ENV_VS,
  TAG_LFO_FREQ,
  TAG_LFO_AMP,
  TAG_LFO_VS,
  TAG_LFO_TYPE,
  TAG_SELECTED_GENERATOR,  // valid only with "0" for generator and TARGET_NA
  TAG_MOD_TYPE,
  TAG_NUM_TAGS
};


constexpr const char *kParamNames[]{
    "SELECT",  "TOGGLE", "OSC",      "ENV_A",         "ENV_AL",
    "ENV_D",   "ENV_S",  "ENV_R",    "ENV_VS",        "LFO_FREQ",
    "LFO_AMP", "LFO_VS", "LFO_TYPE", "SELECTED_GEN#", "MOD_TYPE"};

enum TargetTag {
  TARGET_NA,
  TARGET_C,
  TARGET_A,
  TARGET_M,
  TARGET_K,
  TARGET_R,
  TARGET_S,
  TARGET_PORTAMENTO,
  NUM_TARGETS
};

constexpr const char *kTargetNames[]{"NONE", "C", "A", "M",
                                     "K",    "R", "S", "PORTAMENTO"};

constexpr const TargetTag kModulationTargets[]{
    TARGET_C, TARGET_A, TARGET_M, TARGET_K, TARGET_R, TARGET_S,
};

// Composed of both param and subparam, key for patch param lookup
struct ParamKey {
  ParamTag p_tag;
  TargetTag target;

  bool operator==(const ParamKey &k) const {
    return k.p_tag == p_tag && k.target == target;
  }

  struct Hash {
    size_t operator()(const ParamKey &k) const {
      return std::hash<ParamTag>()(k.p_tag) ^ std::hash<TargetTag>()(k.target);
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


}  // namespace sidebands