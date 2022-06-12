cmake_minimum_required(VERSION 3.21.0)

set(CMAKE_CXX_STANDARD 20)
set(ABSL_PROPAGATE_CXX_STD ON)

project(sidebands)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(NuGet)

# Third party packages are pulled in via FetchContent
include(FetchContent)

# VST3SDK
FetchContent_Declare(
        vst3sdk
        GIT_REPOSITORY https://github.com/steinbergmedia/vst3sdk.git
        GIT_SHALLOW 1
)
FetchContent_MakeAvailable(vst3sdk)

#set(SMTG_CREATE_BUNDLE_FOR_WINDOWS OFF)
set(SMTG_ENABLE_TARGET_VARS_LOG ON)

# The samples were useful for looking at, but now they're bloating the build dir.
set(SMTG_ADD_VST3_PLUGINS_SAMPLES OFF)
set(SMTG_ADD_VST3_HOSTING_SAMPLES OFF)

set(SMTG_CREATE_PLUGIN_LINK ON)

# If validation is on then the execution of the host in debug seems to fail
set(SMTG_RUN_VST_VALIDATOR OFF)

# We don't want VSTGUI to bloat the binary.
set(SMTG_ADD_VSTGUI OFF)

# Abseil (and probably others) will fail in VST load if shared libs are on.
set(BUILD_SHARED_LIBS FALSE)

# Abseil link on Linux will fail without this.
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(SMTG_CREATE_MODULE_INFO OFF)

if (NOT vst3sdk_SOURCE_DIR)
    message(FATAL_ERROR "Path to VST3 SDK is empty!")
endif ()

# Glog
FetchContent_Declare(
        glog
        GIT_REPOSITORY https://github.com/google/glog.git
        GIT_SHALLOW 1
)
FetchContent_MakeAvailable(glog)

# Abseil
FetchContent_Declare(
        abseil
        GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
        GIT_TAG c2ef7033380a3d8661fee76465097422170fb653
)
FetchContent_MakeAvailable(abseil)

# vectorclass
FetchContent_Declare(
        vectorclass
        GIT_REPOSITORY https://github.com/vectorclass/version2.git
        GIT_SHALLOW 1
)
FetchContent_MakeAvailable(vectorclass)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        release-1.11.0
        GIT_SHALLOW 1
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(BUILD_GTEST ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# vstwebview
FetchContent_Declare(
        vstwebview
        GIT_REPOSITORY https://github.com/rdaum/vstwebview.git
        GIT_TAG main
)
FetchContent_MakeAvailable(vstwebview)

# sigslot
FetchContent_Declare(
        sigslot
        GIT_REPOSITORY https://github.com/palacaze/sigslot.git
)
FetchContent_MakeAvailable(sigslot)

IF (WIN32)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  /fp:fast /arch:AVX2 /DINSTRSET=7 /await")
ELSE()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -ffast-math -mavx2 -fabi-version=0")
    set(PLATFORM_LIBRARIES tbb)
ENDIF()

smtg_enable_vst3_sdk()
smtg_add_vst3plugin(sidebands
        source/constants.h
        source/globals.h
        source/globals.cc
        source/tags.h
        source/tags.cc
        source/version.h
        source/sidebands_cids.h
        source/sidebands_entry.cc

        source/processor/sidebands_processor.h
        source/processor/sidebands_processor.cc
        source/processor/patch_processor.h
        source/processor/patch_processor.cc
        source/processor/events.h

        source/processor/util/processor_param_value.h
        source/processor/util/parameter.cc
        source/processor/util/parameter.h
        source/processor/util/sample_accurate_value.h
        source/processor/util/sample_accurate_value.cc
        source/processor/util/parameter.cc
        source/processor/util/parameter.h
        source/processor/util/processor_param_value.h

        source/dsp/oscbuffer.cc
        source/dsp/oscbuffer.h
        source/dsp/integrator.h
        source/dsp/integrator.cc
        source/dsp/dc_block.h
        source/dsp/dc_block.cc
        source/dsp/fft.cc
        source/dsp/fft.h

        source/processor/synthesis/envgen.h
        source/processor/synthesis/envgen.cc
        source/processor/synthesis/lfo.h
        source/processor/synthesis/lfo.cc
        source/processor/synthesis/oscillator.h
        source/processor/synthesis/oscillator.cc
        source/processor/synthesis/generator.h
        source/processor/synthesis/generator.cc
        source/processor/synthesis/player.h
        source/processor/synthesis/player.cc
        source/processor/synthesis/voice.h
        source/processor/synthesis/voice.cc
        source/processor/synthesis/modulation_source.h

        source/controller/sidebands_controller.h
        source/controller/sidebands_controller.cc
        source/controller/patch_controller.h
        source/controller/patch_controller.cc

        source/controller/webui/index.ts
        source/controller/webui/view/pureknob.ts
        source/controller/webui/view/templates.tsx
        source/controller/webui/view/controls.ts
        source/controller/webui/view/envelope_editor_view.ts
        source/controller/webui/view/generator_tab_view.ts
        source/controller/webui/view/main_view.ts
        source/controller/webui/view/views.ts
        source/controller/webui/model/sidebands_model.ts
        source/controller/webui/model/vst_model.ts
        source/controller/webui/style.css
        source/controller/webui/index.html
        )

add_custom_target(npm-dependencies-install
        COMMAND npm install -D
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/source/controller/webui)
add_custom_target(perform-webpack
        COMMAND npx webpack --config ${CMAKE_SOURCE_DIR}/source/controller/webui/webpack.config.js --env outdir=${CMAKE_BINARY_DIR}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/source/controller/webui)
add_dependencies(perform-webpack npm-dependencies-install)
add_dependencies(sidebands perform-webpack)
add_dependencies(sidebands vstwebview)

smtg_target_add_plugin_resource(sidebands source/controller/webui/index.html)
smtg_target_add_plugin_resource(sidebands source/controller/webui/style.css)
smtg_target_add_plugin_resource(sidebands resource/MetalSwitchOff.png)
smtg_target_add_plugin_resource(sidebands resource/MetalSwitchOn.png)
smtg_target_add_plugin_resource(sidebands ${CMAKE_CURRENT_BINARY_DIR}/dist/main.js)

smtg_target_add_plugin_snapshots(sidebands
        RESOURCES
        resource/7A5DD92AB0615F028293F8308139D3CE_snapshot.png
        resource/7A5DD92AB0615F028293F8308139D3CE_snapshot_2.0x.png
        )

target_link_libraries(sidebands
        PRIVATE
        vstwebview
        sdk
        glog
        absl::strings
        absl::statusor
        nlohmann_json::nlohmann_json
        ${PLATFORM_LIBRARIES}
)
target_include_directories(sidebands PRIVATE source ${vectorclass_SOURCE_DIR} ${sigslot_SOURCE_DIR}/include)

if (SMTG_MAC)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 10.12)
    smtg_target_set_bundle(sidebands
            BUNDLE_IDENTIFIER com.thimbleware.sidebands
            INFOPLIST "${CMAKE_CURRENT_LIST_DIR}/resource/Info.plist" PREPROCESS
            )
    smtg_target_set_debug_executable(sidebands
            "/Applications/VST3PluginTestHost.app"
            "--pluginfolder;$(BUILT_PRODUCTS_DIR)"
            )
elseif (SMTG_WIN)
    target_sources(sidebands PRIVATE
            resource/win32resource.rc
            )
    if (MSVC)
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT sidebands)

        smtg_target_set_debug_executable(sidebands
                "$(ProgramW6432)/Steinberg/VST3PluginTestHost/VST3PluginTestHost.exe"
                "--pluginfolder \"$(OutDir)/\""
                )
    endif ()
endif (SMTG_MAC)

