#pragma once
#include "Stratum.h"
#include "Records.h"

namespace Stratum::Logging {
    class STRATUM_API ISink {
    public:
        virtual void sink(std::string_view v_Payload) = 0;
        virtual ~ISink() = default;

        ISink()                              = default;
        ISink(const ISink&)                  = delete;
        ISink& operator=(const ISink&)       = delete;
        ISink(ISink&&)            noexcept   = default;
        ISink& operator=(ISink&&) noexcept   = default;
    };

    class STRATUM_API ISinkRouter {
    public:
        virtual void write(const Records::LogEntry&       ro_Entry) = 0;
        virtual void write(const Records::ExceptionEntry& ro_Entry) = 0;
        virtual void write(const Records::TracerEntry&    ro_Entry) = 0;
        virtual ~ISinkRouter() = default;

        ISinkRouter()                                    = default;
        ISinkRouter(const ISinkRouter&)                  = delete;
        ISinkRouter& operator=(const ISinkRouter&)       = delete;
        ISinkRouter(ISinkRouter&&)            noexcept   = default;
        ISinkRouter& operator=(ISinkRouter&&) noexcept   = default;
    };
}
