//------------------------------------------------------------------------
// Copyright(c) 2022 My Plug-in Company.
//------------------------------------------------------------------------

#pragma once

#include "pluginterfaces/base/fplatform.h"

// Here you can define the version of your plug-in: "Major.Sub.Release.Build""
#define MAJOR_VERSION_STR "1"
#define MAJOR_VERSION_INT 1

#define SUB_VERSION_STR "0"
#define SUB_VERSION_INT 0

#define RELEASE_NUMBER_STR "0"
#define RELEASE_NUMBER_INT 0

#define BUILD_NUMBER_STR \
  "1"  // Build number to be sure that each result could be identified.
#define BUILD_NUMBER_INT 1

// Version with build number (example "1.0.3.342")
#define FULL_VERSION_STR                                       \
  MAJOR_VERSION_STR "." SUB_VERSION_STR "." RELEASE_NUMBER_STR \
                    "." BUILD_NUMBER_STR

// Version without build number (example "1.0.3")
#define VERSION_STR MAJOR_VERSION_STR "." SUB_VERSION_STR "." RELEASE_NUMBER_STR

#define stringOriginalFilename "sidebands.vst3"
#if SMTG_PLATFORM_64
#define stringFileDescription "sidebands VST3 (64Bit)"
#else
#define stringFileDescription "sidebands VST3"
#endif
#define stringCompanyName "Thimbleware\0"
#define stringLegalCopyright "Copyright(c) 2022 Thimbleware."
#define stringLegalTrademarks \
  "VST is a trademark of Steinberg Media Technologies GmbH"
