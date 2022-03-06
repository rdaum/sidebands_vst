#pragma once

#include "pluginterfaces/base/funknown.h"
#include "pluginterfaces/vst/vsttypes.h"

namespace sidebands {
//------------------------------------------------------------------------
static const Steinberg::FUID kSidebandsProcessorUID(0x7A5DD92A, 0xB0615F02,
                                                    0x8293F830, 0x8139D3CE);
static const Steinberg::FUID kSidebandsControllerUID(0x8AF5F67B, 0x3AED5632,
                                                     0xBBDAF45F, 0xD48F7FB0);

#define sidebandsVST3Category "Instrument|Synth"

//------------------------------------------------------------------------
}  // namespace sidebands
