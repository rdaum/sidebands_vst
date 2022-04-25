#include "controller/sidebands_controller.h"
#include "processor/sidebands_processor.h"
#include "public.sdk/source/main/pluginfactory.h"
#include "sidebands_cids.h"
#include "version.h"

#define stringPluginName "Sidebands"

using namespace Steinberg::Vst;
using namespace sidebands;

BEGIN_FACTORY_DEF("Daum Audio Works", "https://www.mycompanyname.com",
                  "mailto:info@mycompanyname.com")

// Processor.
// A kVstAudioEffectClass component
DEF_CLASS2(
    INLINE_UID_FROM_FUID(kSidebandsProcessorUID),
    PClassInfo::kManyInstances,  // cardinality
    kVstAudioEffectClass,        // the component category (do not changed this)
    stringPluginName,            // here the Plug-in name (to be changed)
    Vst::kDistributable,         // means that component and controller could be
                                 // distributed on different computers
    sidebandsVST3Category,
    FULL_VERSION_STR,   // Plug-in version (to be changed)
    kVstVersionString,  // the VST 3 SDK version (do not changed this, use
                        // always this define)
    SidebandsProcessor::Instantiate)  // function pointer called when this
                                      // component should be instantiated

// Controller
// A kVstComponentControllerClass component
DEF_CLASS2(
    INLINE_UID_FROM_FUID(kSidebandsControllerUID),
    PClassInfo::kManyInstances,    // cardinality
    kVstComponentControllerClass,  // the Controller category (do not changed
                                   // this)
    stringPluginName
    "Controller",       // controller name (could be the same as component name)
    0,                  // not used here
    "",                 // not used here
    FULL_VERSION_STR,   // Plug-in version (to be changed)
    kVstVersionString,  // the VST 3 SDK version (do not changed this, use
                        // always this define)
    SidebandsController::Instantiate)  // function pointer called when this
                                       // component should be instantiated

END_FACTORY
