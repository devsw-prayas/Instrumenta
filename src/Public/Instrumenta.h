#pragma once
#ifndef INSTRUMENTA
#define INSTRUMENTA __declspec(dllexport)
#endif

#ifndef GENERIC_CONSOLE_OUTPUT_TAG
#define GENERIC_CONSOLE_OUTPUT_TAG "console"
#endif

#ifndef GENERIC_FILE_OUTPUT_TAG
#define GENERIC_FILE_OUTPUT_TAG "file"
#endif

#ifndef GENERIC_BUFFERED_OUTPUT_TAG
#define GENERIC_BUFFERED_OUTPUT_TAG "buffered"
#endif

#ifndef CONSOLE_SINK_ROUTER_TAG
#define CONSOLE_SINK_ROUTER_TAG "console_router"
#endif

#ifndef FILE_SINK_ROUTER_TAG
#define FILE_SINK_ROUTER_TAG "file_router"
#endif

#ifndef BUFFERED_SINK_ROUTER_TAG
#define BUFFERED_SINK_ROUTER_TAG "buffered_router"
#endif

inline bool isInstrumentationInitialized = false;