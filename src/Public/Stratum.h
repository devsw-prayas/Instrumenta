#pragma once
#include <cstdint>
#include <chrono>
#include <source_location>


#ifndef STRATUM
#define STRATUM __declspec(dllexport)
#endif

#ifndef GENERIC_CONSOLE_OUTPUT_TAG
#define GENERIC_CONSOLE_OUTPUT_TAG "console"
#endif

#ifndef GENERIC_FILE_OUTPUT_TAG
#define GENERIC_FILE_OUTPUT_TAG "file"
#endif

#ifndef CONSOLE_SINK_ROUTER_TAG
#define CONSOLE_SINK_ROUTER_TAG "console_router"
#endif

#ifndef FILE_SINK_ROUTER_TAG
#define FILE_SINK_ROUTER_TAG "file_router"
#endif

inline bool isInstrumentationInitialized = false;