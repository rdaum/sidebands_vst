cmake_minimum_required(VERSION 3.21.0)

set(CMAKE_CXX_STANDARD 20)
set(ABSL_PROPAGATE_CXX_STD ON)

project(sidebands)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

include(NuGet)

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

# vectorclass
FetchContent_Declare(
        vectorclass
        GIT_REPOSITORY https://github.com/vectorclass/version2.git
)
FetchContent_MakeAvailable(vectorclass)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        release-1.11.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
set(BUILD_GTEST ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# json
FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.10.5/json.tar.xz)
FetchContent_MakeAvailable(json)

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

IF (WIN32)
    set(WEBVIEW_PLATFORM_SOURCES
            source/controller/webview/win32/webview_win32.h
            source/controller/webview/win32/webview_win32.cc
            source/controller/webview/win32/webview_edge_chromium.h
            source/controller/webview/win32/webview_edge_chromium.cc)
ENDIF()

if(UNIX AND NOT APPLE)
    set(WEBVIEW_PLATFORM_SOURCES
            source/controller/webview/gtk/webview_gtk.h)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
    pkg_check_modules(GTKMM3 REQUIRED gtkmm-3.0)
    pkg_check_modules(WEBKIT2GTK REQUIRED webkit2gtk-4.0 )
endif()

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

        source/processor/util/sample_accurate_value.h
        source/processor/util/sample_accurate_value.cc

        source/dsp/oscbuffer.cc
        source/dsp/oscbuffer.h
        source/dsp/integrator.h
        source/dsp/integrator.cc
        source/dsp/dc_block.h
        source/dsp/dc_block.cc

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

        source/controller/webview/webview.h
        source/controller/webview/webview.cc
        "${WEBVIEW_PLATFORM_SOURCES}"

        source/controller/webview_pluginview.h
        source/controller/webview_pluginview.cc
        source/controller/webview_controller_bindings.cc
        source/controller/webview_controller_bindings.h

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


# Needed for webview2.h & friends on Win32.
if (WIN32)
    option(WIL_BUILD_PACKAGING "" OFF)
    option(WIL_BUILD_TESTS  "" OFF)
    FetchContent_Declare(wil GIT_REPOSITORY "https://github.com/microsoft/wil")
    FetchContent_MakeAvailable(wil)

    nuget_add(WebView2 "Microsoft.Web.WebView2" 1.0.1150.38)
    nuget_add(CppWinRT "Microsoft.Windows.CppWinRT" 2.0.220315.1)

    target_compile_definitions(sidebands PRIVATE UNICODE=1 _UNICODE=1)
    target_include_directories(sidebands PRIVATE ${WebView2_PATH}/build/native/include)

    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        target_link_libraries(sidebands PRIVATE ${WebView2_PATH}/build/native/x64/WebView2LoaderStatic.lib)
    else()
        target_link_libraries(sidebands PRIVATE ${WebView2_PATH}/build/native/x86/WebView2LoaderStatic.lib)
    endif()

    target_link_libraries(sidebands PRIVATE WIL::WIL Dwmapi Shlwapi)
endif()

if(UNIX AND NOT APPLE)
    target_include_directories(sidebands PRIVATE ${GTK3_INCLUDE_DIRS} ${WEBKIT2GTK_INCLUDE_DIRS} ${GTKMM3_INCLUDE_DIRS})
    target_link_libraries(sidebands PRIVATE ${GTK3_LIBRARIES} ${WEBKIT2GTK_LIBRARIES} ${GTKMM3_LIBRARIES})
endif()

add_custom_target(npm-dependencies-install
        COMMAND npm install -D
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/source/controller/webui)
add_custom_target(perform-webpack
        COMMAND npx webpack --config ${CMAKE_SOURCE_DIR}/source/controller/webui/webpack.config.js --env outdir=${CMAKE_BINARY_DIR}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/source/controller/webui)
add_dependencies(perform-webpack npm-dependencies-install)
add_dependencies(sidebands perform-webpack)

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
