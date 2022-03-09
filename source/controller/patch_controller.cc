#include "controller/patch_controller.h"

#include <absl/strings/str_format.h>
#include <base/source/fstreamer.h>
#include <pluginterfaces/base/ustring.h>
#include <pluginterfaces/vst/vsttypes.h>
#include <string>

#include "constants.h"
#include "tags.h"

namespace sidebands {

using Steinberg::Vst::ParameterInfo;

IPtr<RangeParameter> BooleanParameter(Steinberg::Vst::UnitID unit_id,
                                      const std::string &name, ParamTag tag,
                                      TargetTag sp, uint32_t gen_num) {
  auto info = ParameterInfo{
      .id = TagFor(gen_num, tag, sp),
      .stepCount = 1,
      .defaultNormalizedValue = 0,
      .unitId = unit_id,
  };
  std::string full_name = absl::StrFormat("Gen %d %s", gen_num, name);
  Steinberg::UString(info.title, USTRINGSIZE(info.title))
      .assign(USTRING(full_name.c_str()));

  return new RangeParameter(info, 0, 1);
}

IPtr<RangeParameter> GenericParameter(Steinberg::Vst::UnitID unit_id,
                                      const std::string &name, ParamTag tag,
                                      TargetTag sp, uint32_t gen_num,
                                      ParamValue min, ParamValue max) {
  std::string full_name = absl::StrFormat("Gen %d %s", gen_num, name);

  auto info = ParameterInfo{
      .id = TagFor(gen_num, tag, sp),
      .stepCount = 0,
      .defaultNormalizedValue = 0,
      .unitId = unit_id,
  };
  Steinberg::UString(info.title, USTRINGSIZE(info.title))
      .assign(USTRING(full_name.c_str()));

  return new RangeParameter(info, min, max);
}

IPtr<RangeParameter> ModTypeParameter(Steinberg::Vst::UnitID unit_id,
                                      TargetTag target, uint32_t gen_num) {
  std::string full_name =
      absl::StrFormat("Gen %d Mod Type %s", gen_num, kTargetNames[target]);

  auto info = ParameterInfo{
      .id = TagFor(gen_num, TAG_MOD_TYPE, target),
      .stepCount = 1,
      .defaultNormalizedValue = 0,
      .unitId = unit_id,
  };
  Steinberg::UString(info.title, USTRINGSIZE(info.title))
      .assign(USTRING(full_name.c_str()));

  return new RangeParameter(info, 0, kNumModTypes - 1);
}

IPtr<RangeParameter> OscillatorParameter(Steinberg::Vst::UnitID unit_id,
                                         const std::string &name, TargetTag sp,
                                         uint32_t gen_num, ParamValue min,
                                         ParamValue max) {
  std::string full_name =
      absl::StrFormat("Gen %d Oscillator %s", gen_num, name);

  auto info = ParameterInfo{
      .id = TagFor(gen_num, TAG_OSC, sp),
      .stepCount = 0,
      .defaultNormalizedValue = 0,
      .unitId = unit_id,
  };
  Steinberg::UString(info.title, USTRINGSIZE(info.title))
      .assign(USTRING(full_name.c_str()));

  return new RangeParameter(info, min, max);
}

IPtr<RangeParameter> LFOParameter(Steinberg::Vst::UnitID unit_id,
                                  const std::string &name, ParamTag tag,
                                  TargetTag sp, uint32_t gen_num,
                                  ParamValue min, ParamValue max) {
  std::string full_name = absl::StrFormat("Gen %d LFO %s", gen_num, name);

  auto info = ParameterInfo{
      .id = TagFor(gen_num, tag, sp),
      .stepCount = 0,
      .defaultNormalizedValue = 0,
      .unitId = unit_id,
  };
  Steinberg::UString(info.title, USTRINGSIZE(info.title))
      .assign(USTRING(full_name.c_str()));

  return new RangeParameter(info, min, max);
}

IPtr<RangeParameter> EnvelopeParameter(Steinberg::Vst::UnitID unit_id,
                                       const std::string &name, ParamTag tag,
                                       TargetTag sp, uint32_t gen_num,
                                       ParamValue min, ParamValue max) {
  std::string full_name = absl::StrFormat("Gen %d Envelope %s", gen_num, name);

  auto info = ParameterInfo{
      .id = TagFor(gen_num, tag, sp),
      .stepCount = 0,
      .defaultNormalizedValue = 0,
      .unitId = unit_id,
  };
  Steinberg::UString(info.title, USTRINGSIZE(info.title))
      .assign(USTRING(full_name.c_str()));

  return new RangeParameter(info, min, max);
}

void PatchController::AppendParameters(ParameterContainer *container) {

  for (int generator = 0; generator < kNumGenerators; generator++) {
    auto unit_id = MakeUnitID(UNIT_GENERATOR, generator);
    container->addParameter(BooleanParameter(
        unit_id, "On", TAG_GENERATOR_TOGGLE, TARGET_NA, generator));
    container->addParameter(
        OscillatorParameter(unit_id, "C", TARGET_C, generator, 0, 8));
    container->addParameter(
        OscillatorParameter(unit_id, "A", TARGET_A, generator, 0, 1));
    container->addParameter(
        OscillatorParameter(unit_id, "M", TARGET_M, generator, 0, 8));
    container->addParameter(
        OscillatorParameter(unit_id, "K", TARGET_K, generator, 0, 10));
    container->addParameter(
        OscillatorParameter(unit_id, "R", TARGET_R, generator, 0, 1));
    container->addParameter(
        OscillatorParameter(unit_id, "S", TARGET_S, generator, -1, 1));
    container->addParameter(OscillatorParameter(
        unit_id, "Portamento", TARGET_PORTAMENTO, generator, 0, 1));

    for (auto target : kModulationTargets) {
      container->addParameter(ModTypeParameter(unit_id, target, generator));

      const std::string env_name =
          absl::StrFormat("%s Env ", kTargetNames[target]);
      container->addParameter(EnvelopeParameter(
          unit_id, env_name + " A", TAG_ENV_A, target, generator, 0, 5));
      container->addParameter(EnvelopeParameter(
          unit_id, env_name + " AL", TAG_ENV_AL, target, generator, 0, 1));
      container->addParameter(EnvelopeParameter(
          unit_id, env_name + " D", TAG_ENV_D, target, generator, 0, 5));
      container->addParameter(EnvelopeParameter(
          unit_id, env_name + " S", TAG_ENV_S, target, generator, 0, 1));
      container->addParameter(EnvelopeParameter(
          unit_id, env_name + " R", TAG_ENV_R, target, generator, 0, 1));
      container->addParameter(EnvelopeParameter(
          unit_id, env_name + " VS", TAG_ENV_VS, target, generator, 0, 1));

      container->addParameter(LFOParameter(unit_id, "Type", TAG_LFO_TYPE,
                                           target, generator, 0,
                                           kNumLFOTypes - 1));
      container->addParameter(LFOParameter(unit_id, "Amplitude", TAG_LFO_AMP,
                                           target, generator, 0, 1));
      container->addParameter(LFOParameter(unit_id, "Frequency", TAG_LFO_FREQ,
                                           target, generator, 0, 20));
      container->addParameter(LFOParameter(unit_id, "VelSense", TAG_LFO_VS,
                                           target, generator, 0, 1));
    }
  }
}

Steinberg::tresult
PatchController::LoadPatch(Steinberg::IBStream *stream,
                           Steinberg::Vst::IEditController *edit_controller) {
  Steinberg::IBStreamer streamer(stream);
  while (true) {
    Steinberg::Vst::ParamID id;
    if (!streamer.readInt32u(id)) break;
    ParamValue v;
    CHECK(streamer.readDouble(v)) << "Unable to read value for param id: " << id;
    auto p = edit_controller->normalizedParamToPlain(id, v);
    LOG(INFO) << "Setting parameter: " << TagStr(id) << " to normalized: " << v
              << " (plain: " << p << ")";
    if (TargetFor(id) == TARGET_C && GeneratorFor(id) == 0 && ParamFor(id) == TAG_OSC) {
      LOG(INFO) << "C";
    }
    edit_controller->setParamNormalized(id, v);
  }
  return Steinberg::kResultOk;
}

Steinberg::tresult PatchController::SavePatch(Steinberg::IBStream *stream) {

  return Steinberg::kResultOk;
}

} // namespace sidebands