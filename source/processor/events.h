#pragma once

#include <sigslot/signal.hpp>

#include "tags.h"

namespace sidebands {

struct PlayerEvents {
  sigslot::signal<int /*note id*/, int /* gennum */, TargetTag,
                  off_t /* stage*/>
      EnvelopeStageChange;
};

class Voice;
struct VoiceEvents {
  sigslot::signal<Voice *> VoiceOn;
  sigslot::signal<Voice *> VoiceRelease;
  sigslot::signal<Voice *> VoiceOff;

  sigslot::signal<Voice *, int /* gennum */, TargetTag, off_t>
      EnvelopeStageChange;
};

class Generator;
struct GeneratorEvents {
  sigslot::signal<Generator *> GeneratorOn;
  sigslot::signal<Generator *> GeneratorRelease;
  sigslot::signal<Generator *> GeneratorOff;

  sigslot::signal<int /* gennum */, TargetTag, off_t> EnvelopeStageChange;
};

struct EnvelopeEvents {
  sigslot::signal<> Start;
  sigslot::signal<> Release;
  sigslot::signal<off_t> StageChange;
  sigslot::signal<> Done;
};

}  // namespace sidebands