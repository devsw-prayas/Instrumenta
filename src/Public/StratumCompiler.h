#pragma once

#if defined(_MSC_VER)
    #define STRATUM_COMPILER_MSVC  1
#else
    #define STRATUM_COMPILER_MSVC  0
#endif

#if defined(__clang__)
    #define STRATUM_COMPILER_CLANG 1
#else
    #define STRATUM_COMPILER_CLANG 0
#endif

#if defined(__GNUC__) && !defined(__clang__)
    #define STRATUM_COMPILER_GCC   1
#else
    #define STRATUM_COMPILER_GCC   0
#endif

#if STRATUM_COMPILER_MSVC
    #define STRATUM_FORCEINLINE __forceinline
    #define STRATUM_NOINLINE    __declspec(noinline)
#elif STRATUM_COMPILER_CLANG || STRATUM_COMPILER_GCC
    #define STRATUM_FORCEINLINE inline __attribute__((always_inline))
    #define STRATUM_NOINLINE    __attribute__((noinline))
#else
    #define STRATUM_FORCEINLINE inline
    #define STRATUM_NOINLINE
#endif

#define STRATUM_INLINE inline

#ifndef STRATUM_API
#if STRATUM_COMPILER_MSVC
    #define STRATUM_API __declspec(dllexport)
#elif STRATUM_COMPILER_CLANG || STRATUM_COMPILER_GCC
    #define STRATUM_API __attribute__((visibility("default")))
#else
    #define STRATUM_API
#endif
#endif

#if STRATUM_COMPILER_MSVC
    #define STRATUM_COMPILER_BARRIER() _ReadWriteBarrier()
#elif STRATUM_COMPILER_CLANG || STRATUM_COMPILER_GCC
    #define STRATUM_COMPILER_BARRIER() asm volatile("" ::: "memory")
#else
    #define STRATUM_COMPILER_BARRIER()
#endif

#if STRATUM_COMPILER_MSVC
    #define STRATUM_OPTIMIZE_OFF __pragma(optimize("", off))
    #define STRATUM_OPTIMIZE_ON  __pragma(optimize("", on))
#elif STRATUM_COMPILER_CLANG
    #define STRATUM_OPTIMIZE_OFF _Pragma("clang optimize off")
    #define STRATUM_OPTIMIZE_ON  _Pragma("clang optimize on")
#elif STRATUM_COMPILER_GCC
    #define STRATUM_OPTIMIZE_OFF _Pragma("GCC optimize(\"O0\")")
    #define STRATUM_OPTIMIZE_ON  _Pragma("GCC optimize(\"O2\")")
#else
    #define STRATUM_OPTIMIZE_OFF
    #define STRATUM_OPTIMIZE_ON
#endif

#if STRATUM_COMPILER_CLANG || STRATUM_COMPILER_GCC
    #define STRATUM_LIKELY(x)   __builtin_expect(!!(x), 1)
    #define STRATUM_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
    #define STRATUM_LIKELY(x)   (x)
    #define STRATUM_UNLIKELY(x) (x)
#endif

#if STRATUM_COMPILER_MSVC
    #define STRATUM_DEBUG_BREAK() __debugbreak()
    #define STRATUM_TRAP()        __debugbreak()
#elif STRATUM_COMPILER_CLANG || STRATUM_COMPILER_GCC
    #define STRATUM_DEBUG_BREAK() __builtin_trap()
    #define STRATUM_TRAP()        __builtin_trap()
#else
    #include <cstdlib>
    #define STRATUM_DEBUG_BREAK() std::abort()
    #define STRATUM_TRAP()        std::abort()
#endif

#if STRATUM_COMPILER_MSVC
    #define STRATUM_UNREACHABLE() __assume(0)
#elif STRATUM_COMPILER_CLANG || STRATUM_COMPILER_GCC
    #define STRATUM_UNREACHABLE() __builtin_unreachable()
#else
    #define STRATUM_UNREACHABLE() STRATUM_TRAP()
#endif

#if STRATUM_COMPILER_MSVC
    #define STRATUM_PRAGMA(x) __pragma(x)
#elif STRATUM_COMPILER_CLANG || STRATUM_COMPILER_GCC
    #define STRATUM_PRAGMA(x) _Pragma(#x)
#else
    #define STRATUM_PRAGMA(x)
#endif

#if STRATUM_COMPILER_MSVC
    #define STRATUM_DIAGNOSTIC_PUSH    STRATUM_PRAGMA(warning(push))
    #define STRATUM_DIAGNOSTIC_POP     STRATUM_PRAGMA(warning(pop))
    #define STRATUM_DISABLE_WARNING(w) STRATUM_PRAGMA(warning(disable : w))
#elif STRATUM_COMPILER_CLANG
    #define STRATUM_DIAGNOSTIC_PUSH    STRATUM_PRAGMA(clang diagnostic push)
    #define STRATUM_DIAGNOSTIC_POP     STRATUM_PRAGMA(clang diagnostic pop)
    #define STRATUM_DISABLE_WARNING(w) STRATUM_PRAGMA(clang diagnostic ignored w)
#elif STRATUM_COMPILER_GCC
    #define STRATUM_DIAGNOSTIC_PUSH    STRATUM_PRAGMA(GCC diagnostic push)
    #define STRATUM_DIAGNOSTIC_POP     STRATUM_PRAGMA(GCC diagnostic pop)
    #define STRATUM_DISABLE_WARNING(w) STRATUM_PRAGMA(GCC diagnostic ignored w)
#else
    #define STRATUM_DIAGNOSTIC_PUSH
    #define STRATUM_DIAGNOSTIC_POP
    #define STRATUM_DISABLE_WARNING(w)
#endif

#if defined(__has_cpp_attribute)
    #if __has_cpp_attribute(fallthrough)
        #define STRATUM_FALLTHROUGH [[fallthrough]]
    #else
        #define STRATUM_FALLTHROUGH
    #endif
#else
    #define STRATUM_FALLTHROUGH
#endif

#if defined(__has_cpp_attribute)
    #if __has_cpp_attribute(nodiscard)
        #define STRATUM_NODISCARD [[nodiscard]]
        #if __cplusplus >= 202002L
            #define STRATUM_NODISCARD_MSG(msg) [[nodiscard(msg)]]
        #else
            #define STRATUM_NODISCARD_MSG(msg) [[nodiscard]]
        #endif
    #else
        #define STRATUM_NODISCARD
        #define STRATUM_NODISCARD_MSG(msg)
    #endif
#else
    #define STRATUM_NODISCARD
    #define STRATUM_NODISCARD_MSG(msg)
#endif

#if defined(__has_cpp_attribute)
    #if __has_cpp_attribute(maybe_unused)
        #define STRATUM_MAYBE_UNUSED [[maybe_unused]]
    #else
        #define STRATUM_MAYBE_UNUSED
    #endif
#else
    #define STRATUM_MAYBE_UNUSED
#endif

#if defined(__has_cpp_attribute)
    #if __has_cpp_attribute(deprecated)
        #define STRATUM_DEPRECATED         [[deprecated]]
        #define STRATUM_DEPRECATED_MSG(msg) [[deprecated(msg)]]
    #else
        #define STRATUM_DEPRECATED
        #define STRATUM_DEPRECATED_MSG(msg)
    #endif
#else
    #define STRATUM_DEPRECATED
    #define STRATUM_DEPRECATED_MSG(msg)
#endif

#if defined(__has_cpp_attribute)
    #if __has_cpp_attribute(noreturn)
        #define STRATUM_NORETURN [[noreturn]]
    #else
        #define STRATUM_NORETURN
    #endif
#else
    #define STRATUM_NORETURN
#endif

#if STRATUM_COMPILER_MSVC
    #define STRATUM_RESTRICT __restrict
#elif STRATUM_COMPILER_CLANG || STRATUM_COMPILER_GCC
    #define STRATUM_RESTRICT __restrict__
#else
    #define STRATUM_RESTRICT
#endif

#define STRATUM_ALIGNAS(n) alignas(n)

#if STRATUM_COMPILER_CLANG || STRATUM_COMPILER_GCC
    #define STRATUM_ASSUME_ALIGNED(ptr, n) __builtin_assume_aligned((ptr), (n))
#else
    #define STRATUM_ASSUME_ALIGNED(ptr, n) (ptr)
#endif

#if STRATUM_COMPILER_CLANG || STRATUM_COMPILER_GCC
    #define STRATUM_HOT  __attribute__((hot))
    #define STRATUM_COLD __attribute__((cold))
#else
    #define STRATUM_HOT
    #define STRATUM_COLD
#endif
