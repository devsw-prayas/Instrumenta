#pragma once

#if defined(_MSC_VER) && !defined(_ENABLE_EXTENDED_ALIGNED_STORAGE)
    #define _ENABLE_EXTENDED_ALIGNED_STORAGE
#endif

#ifndef STRATUM_API
    #if defined(_WIN32) || defined(__CYGWIN__)
        #if defined(STRATUM_SHARED)
            #define STRATUM_API __declspec(dllexport)
        #else
            #define STRATUM_API __declspec(dllimport)
        #endif
    #elif defined(__GNUC__) || defined(__clang__)
        #define STRATUM_API __attribute__((visibility("default")))
    #else
        #define STRATUM_API
    #endif
#endif

#include <cstdint>
#include <cstddef>
#include <cstring>
#include <chrono>
#include <source_location>
#include <type_traits>
#include <concepts>
#include <array>
#include <string_view>
#include <utility>

#include "StratumDiagnostics.h"

namespace Stratum {

    constexpr size_t COMPONENT_NAME_MAX = 64;
    constexpr size_t MESSAGE_MAX        = 256;
    constexpr size_t LABEL_MAX          = 128;
    constexpr size_t LOGGER_NAME_MAX    = 64;
    constexpr size_t TAG_MAX            = 64;
    constexpr size_t MAX_ROUTES         = 16;
    constexpr size_t MAX_LOGGERS        = 32;

}

#ifndef STRATUM_TAG_CONSOLE
    #define STRATUM_TAG_CONSOLE "console"
#endif

#ifndef STRATUM_TAG_FILE
    #define STRATUM_TAG_FILE "file"
#endif

namespace Stratum {
    inline bool g_InstrumentationInitialized = false;
}
