#include "globals.h"

#include <numeric>

namespace sidebands {

double EnvelopeRampCoefficient(double start_level, double end_level,
                               size_t length_in_samples) {
  return 1.0 + (std::log(end_level) - std::log(start_level)) /
                   ((double)length_in_samples);
}

Steinberg::tresult
SetPlayerStateAttributes(Steinberg::Vst::IAttributeList *attributes,
                         const PlayerState &player_state) {
  return attributes->setBinary(kAttrPlayerStateAttr, &player_state,
                               sizeof(player_state));
}

Steinberg::tresult
GetPlayerStateAttributes(Steinberg::Vst::IAttributeList *attributes,
                         PlayerState *ps) {
  const void* data;
  Steinberg::uint32 size;
  if (attributes->getBinary(kAttrPlayerStateAttr, data, size) != Steinberg::kResultOk ||
      size != sizeof(PlayerState)) {
    return Steinberg::kResultFalse;
  }

  memcpy(ps, data, size);
  return Steinberg::kResultTrue;
}

} // namespace sidebands