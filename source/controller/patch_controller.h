#pragma once

#include <public.sdk/source/vst/vstparameters.h>

namespace sidebands {

using Steinberg::IPtr;
using Steinberg::Vst::Parameter;
using Steinberg::Vst::ParameterContainer;
using Steinberg::Vst::ParameterInfo;
using Steinberg::Vst::ParamID;
using Steinberg::Vst::ParamValue;
using Steinberg::Vst::RangeParameter;

// The controller side of patch management.
// Declaration of all parameter objects.
class PatchController {
 public:
  void AppendParameters(ParameterContainer *container);
  Steinberg::tresult LoadPatch(
      Steinberg::IBStream *stream,
      Steinberg::Vst::IEditController *edit_controller);
  Steinberg::tresult SavePatch(Steinberg::IBStream *stream);

};  // namespace PatchController

}  // namespace sidebands