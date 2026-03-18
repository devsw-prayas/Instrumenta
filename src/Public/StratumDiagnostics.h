#pragma once
#include "StratumCompiler.h"

#if defined(_DEBUG) || defined(DEBUG)
    #define STRATUM_BUILD_DEBUG   1
    #define STRATUM_BUILD_RELEASE 0
#else
    #define STRATUM_BUILD_DEBUG   0
    #define STRATUM_BUILD_RELEASE 1
#endif

#if STRATUM_BUILD_DEBUG
    #define STRATUM_ASSERT(expr)        \
        do {                            \
            if (STRATUM_UNLIKELY(!(expr))) {   \
                STRATUM_DEBUG_BREAK();  \
                STRATUM_TRAP();         \
            }                           \
        } while (0)
#else
    #define STRATUM_ASSERT(expr) do { (void)sizeof(expr); } while (0)
#endif

#if STRATUM_BUILD_DEBUG
    #define STRATUM_ASSUME(expr) STRATUM_ASSERT(expr)
#else
    #if STRATUM_COMPILER_MSVC
        #define STRATUM_ASSUME(expr) __assume(expr)
    #elif STRATUM_COMPILER_CLANG || STRATUM_COMPILER_GCC
        #define STRATUM_ASSUME(expr) do { if (!(expr)) __builtin_unreachable(); } while (0)
    #else
        #define STRATUM_ASSUME(expr) do {} while (0)
    #endif
#endif

#if STRATUM_BUILD_DEBUG
    #define STRATUM_DEBUG_ASSERT(expr) STRATUM_ASSERT(expr)
    #define STRATUM_DEBUG_ASSUME(expr) STRATUM_ASSUME(expr)
#else
    #define STRATUM_DEBUG_ASSERT(expr) do {} while (0)
    #define STRATUM_DEBUG_ASSUME(expr) do {} while (0)
#endif

#define STRATUM_STATIC_ASSERT(expr, msg) static_assert(expr, msg)

#define STRATUM_UNUSED(x) (void)(x)
