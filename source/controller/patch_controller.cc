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
  std::string full_name = absl::StrFormat("Gen %d Mod Types %s (bitset)",
                                          gen_num, kTargetNames[target]);

  auto info = ParameterInfo{
      .id = TagFor(gen_num, TAG_MODULATIONS, target),
      .stepCount = 0,
      .defaultNormalizedValue = 0,
      .unitId = unit_id,
  };
  Steinberg::UString(info.title, USTRINGSIZE(info.title))
      .assign(USTRING(full_name.c_str()));

  return new RangeParameter(info, 0, 255);
}

IPtr<RangeParameter> OscillatorParameter(Steinberg::Vst::UnitID unit_id,
                                         const std::string &name, TargetTag sp,
                                         uint32_t gen_num, ParamValue min,
                                         ParamValue max) {
  std::string full_name =
      absl::StrFormat("Gen %d ModFMOscillator %s", gen_num, name);

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
    container->addParameter(OscillatorParameter(
        unit_id, "Oscillator type", TARGET_OSC_TYPE, generator, 0, 1));

    for (auto target : kModulationTargets) {
      container->addParameter(ModTypeParameter(unit_id, target, generator));

      const std::string env_name =
          absl::StrFormat("%s Env ", kTargetNames[target]);
      container->addParameter(EnvelopeParameter(
          unit_id, env_name + " HT", TAG_ENV_HT, target, generator, 0, 1));

      container->addParameter(EnvelopeParameter(
          unit_id, env_name + " AR", TAG_ENV_AR, target, generator, 0, 1));
      container->addParameter(EnvelopeParameter(
          unit_id, env_name + " AL", TAG_ENV_AL, target, generator, 0, 1));

      container->addParameter(EnvelopeParameter(
          unit_id, env_name + " DR1", TAG_ENV_DR1, target, generator, 0, 1));
      container->addParameter(EnvelopeParameter(
          unit_id, env_name + " DL1", TAG_ENV_DL1, target, generator, 0, 1));

      container->addParameter(EnvelopeParameter(
          unit_id, env_name + " DR2", TAG_ENV_DR2, target, generator, 0, 1));
      container->addParameter(EnvelopeParameter(
          unit_id, env_name + " SL", TAG_ENV_SL, target, generator, 0, 1));

      container->addParameter(EnvelopeParameter(
          unit_id, env_name + " RR1", TAG_ENV_RR1, target, generator, 0, 1));
      container->addParameter(EnvelopeParameter(
          unit_id, env_name + " RL1", TAG_ENV_RL1, target, generator, 0, 1));

      container->addParameter(EnvelopeParameter(
          unit_id, env_name + " RR2", TAG_ENV_RR2, target, generator, 0, 1));

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

Steinberg::tresult PatchController::LoadPatch(
    Steinberg::IBStream *stream,
    Steinberg::Vst::IEditController *edit_controller) {
  Steinberg::IBStreamer streamer(stream);

  LOG(INFO) << "Loading from stream...";
  Steinberg::uint32 num_generators;
  if (!streamer.readInt32u(num_generators)) {
    LOG(ERROR) << "Could not read patch stream; expected generator count.";
    return Steinberg::kResultFalse;
  }
  if (num_generators != kNumGenerators) {
    LOG(ERROR) << "Incompatible generator count. Got: " << num_generators
               << " expected: " << kNumGenerators;
    return Steinberg::kResultFalse;
  }

  while (num_generators--) {
    Steinberg::uint32 stream_gennum, num_params;
    if (!streamer.readInt32u(stream_gennum)) {
      LOG(ERROR) << "Unable to read start of generator";
      return Steinberg::kResultFalse;
    }
    if (!streamer.readInt32u(num_params)) {
      LOG(ERROR) << "Unable to read parameter count for generator: "
                 << stream_gennum;
      return Steinberg::kResultFalse;
    }
    while (num_params--) {
      Steinberg::Vst::ParamID id;
      if (!streamer.readInt32u(id)) break;
      ParamValue v;
      CHECK(streamer.readDouble(v))
          << "Unable to read value for param id: " << id;
      edit_controller->setParamNormalized(id, v);
    }
  }
  return Steinberg::kResultOk;
}

Steinberg::tresult PatchController::SavePatch(Steinberg::IBStream *stream) {
  return Steinberg::kResultOk;
}

}  // namespace sidebands