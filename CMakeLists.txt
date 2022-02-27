cmake_minimum_required(VERSION 3.21.0)

set(CMAKE_CXX_STANDARD 20)
set(ABSL_PROPAGATE_CXX_STD ON)

project(sidebands)

# Third party packages are pulled in via FetchContent
include(FetchContent)

# Glog
FetchContent_Declare(
        vst3sdk
        GIT_REPOSITORY https://github.com/steinbergmedia/vst3sdk.git
)
FetchContent_MakeAvailable(vst3sdk)

#set(SMTG_CREATE_BUNDLE_FOR_WINDOWS OFF)
set(SMTG_ENABLE_TARGET_VARS_LOG ON)

# Symlink requires manual deletion each build for some reason, so don't bother.
set(SMTG_CREATE_PLUGIN_LINK ON)

# If validation is on then the execution of the host in debug seems to fail
set(SMTG_RUN_VST_VALIDATOR OFF)

# Abseil (and probably others) will fail in VST load if shared libs are on.
set(BUILD_SHARED_LIBS FALSE)

if (NOT vst3sdk_SOURCE_DIR)
    message(FATAL_ERROR "Path to VST3 SDK is empty!")
endif ()

# Glog
FetchContent_Declare(
        glog
        GIT_REPOSITORY https://github.com/google/glog.git
)
FetchContent_MakeAvailable(glog)

# Abseil
FetchContent_Declare(
        abseil
        GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
)
FetchContent_MakeAvailable(abseil)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /fp:fast ")

# VST3 SDK setup
set(SMTG_VSTGUI_ROOT "${vst3sdk_SOURCE_DIR}")
smtg_enable_vst3_sdk()
smtg_add_vst3plugin(modfmi
        source/constants.h
        source/globals.h
        source/tags.h
        source/tags.cc
        source/version.h

        source/sidebands_cids.h
        source/sidebands_processor.h
        source/sidebands_processor.cc
        source/sidebands_controller.h
        source/sidebands_controller.cc
        source/sidebands_entry.cc

        source/synthesis/envgen.h
        source/synthesis/envgen.cc
        source/synthesis/lfo.h
        source/synthesis/lfo.cc
        source/synthesis/patch.h
        source/synthesis/patch.cc
        source/synthesis/oscillator.h
        source/synthesis/oscillator.cc
        source/synthesis/generator.h
        source/synthesis/generator.cc
        source/synthesis/player.h
        source/synthesis/player.cc
        source/synthesis/voice.h
        source/synthesis/voice.cc
        source/synthesis/modulation_source.h

        source/ui/patch_param_view.h
        source/ui/patch_param_view.cc
        source/ui/drawbar_view.h
        source/ui/drawbar_view.cc
        source/ui/modulator_editor_view.h
        source/ui/modulator_editor_view.cc
        source/ui/envelope_editor_view.h
        source/ui/envelope_editor_view.cc
        source/ui/lfo_editor_view.cc
        source/ui/lfo_editor_view.h
        source/ui/generator_editor_view.h
        source/ui/generator_editor_view.cc
        source/ui/parameter_editor_view.h
        source/ui/parameter_editor_view.cc
        source/ui/mod_target_view.cc
        source/ui/mod_target_view.h
        source/ui/graphical_envelope_editor.cc
        source/ui/graphical_envelope_editor.h

        resource/sidebands.uidesc
        )

#- VSTGUI Wanted ----
if (SMTG_ADD_VSTGUI)
    target_sources(modfmi
            PRIVATE
            resource/sidebands.uidesc
            )
    target_link_libraries(modfmi
            PRIVATE
            vstgui_support
            )
    smtg_target_add_plugin_resources(modfmi
            RESOURCES
            "resource/sidebands.uidesc"
            "resource/slider_rail.png"
            "resource/slider_handle.png"
            "resource/switches.png"
            "resource/knob2.png"
            "resource/toggle_switch.png"
            "resource/on-off.png"
            "resource/select.png"
            )
endif (SMTG_ADD_VSTGUI)
# -------------------

smtg_target_add_plugin_snapshots(modfmi
        RESOURCES
        resource/7A5DD92AB0615F028293F8308139D3CE_snapshot.png
        resource/7A5DD92AB0615F028293F8308139D3CE_snapshot_2.0x.png
        )

target_link_libraries(modfmi
        PRIVATE
        sdk
        glog
        absl::statusor
        )
target_include_directories(modfmi PRIVATE source)
if (SMTG_MAC)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.12)
    smtg_target_set_bundle(modfmi
            BUNDLE_IDENTIFIER com.thimbleware.modfmi
            INFOPLIST "${CMAKE_CURRENT_LIST_DIR}/resource/Info.plist" PREPROCESS
            )
    smtg_target_set_debug_executable(modfmi
            "/Applications/VST3PluginTestHost.app"
            "--pluginfolder;$(BUILT_PRODUCTS_DIR)"
            )
elseif (SMTG_WIN)
    target_sources(modfmi PRIVATE
            resource/win32resource.rc
            )
    if (MSVC)
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT modfmi)

        smtg_target_set_debug_executable(modfmi
                "$(ProgramW6432)/Steinberg/VST3PluginTestHost/VST3PluginTestHost.exe"
                "--pluginfolder \"$(OutDir)/\""
                )
    endif ()
endif (SMTG_MAC)
