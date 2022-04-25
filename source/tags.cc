#include "tags.h"

#include <absl/strings/str_format.h>

#include "constants.h"

namespace sidebands {

std::pair<UnitTag, uint16_t> ParseUnitID(Steinberg::Vst::UnitID unit_id) {
  return {static_cast<UnitTag>(unit_id >> 16), unit_id & 0x0000ffff};
}

Steinberg::Vst::UnitID MakeUnitID(UnitTag unit_tag, uint16_t unit_id) {
  return unit_tag << 16 | unit_id;
}

std::string TagStr(Steinberg::Vst::ParamID tag) {
  uint8_t gennum = GeneratorFor(tag);
  CHECK_LT(gennum, kNumGenerators);
  ParamTag param_tag = ParamFor(tag);
  CHECK_LT(param_tag, TAG_NUM_TAGS);
  TargetTag sp(TargetFor(tag));
  CHECK_LT(sp, NUM_TARGETS);
  return absl::StrFormat("%d/%s/%s", gennum, kParamNames[param_tag],
                         kTargetNames[sp]);
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

Steinberg::Vst::ParamID TagFor(int generator, ParamTag param,
                               TargetTag target) {
  Steinberg::Vst::ParamID pid = (generator << 24 | target << 8 | param);
  return pid;
}

ParamKey ParamKeyFor(Steinberg::Vst::ParamID tag) {
  return {ParamFor(tag), TargetFor(tag)};
}

}  // namespace sidebands
