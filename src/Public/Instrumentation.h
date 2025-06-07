#pragma once
#include "LogUtils.h"
#include "Logger.h"
#include "Sinks.h"
#include "Instrumenta.h"

#ifndef LOCATION
#define LOCATION "Line Number: " + std::to_string(__LINE__), "Function call: " + std::string(__FUNCSIG__), "File: " + std::string(__FILE__)
#endif

#include <string>
#include <unordered_map>
#include <vector>
#include <mutex>
#include <memory>

namespace instrumenta {
    class INSTRUMENTA Instrumentation {
        static Instrumentation instance;
        std::unordered_map<std::string, ILogger*> loggerRegistry;
        mutable std::mutex registryMutex;

        Instrumentation() = default;

    public:
        static Instrumentation& getInstance();
        ~Instrumentation() = default;
        Instrumentation(const Instrumentation&) = delete;
        Instrumentation& operator=(const Instrumentation&) = delete;
        Instrumentation(Instrumentation&&) = delete;
        Instrumentation& operator=(Instrumentation&&) = delete;

        void registerLogger(const std::string& name, ILogger* logger);
        ILogger* getLogger(const std::string& name);
        void removeLogger(const std::string& name);
        std::vector<std::string> getRegisteredLoggers() const;

        // Global operations
        void flushAll() const;
        void shutdown();
    };

    // Factory for common sink types
    class INSTRUMENTA LogSinkFactory {
    public:
        static std::unique_ptr<ILogSink> createConsoleSink(bool useColors = true);
        static std::unique_ptr<ILogSink> createFileSink(const std::string& filename, bool autoFlush = true);
        static std::unique_ptr<ILogSink> createBufferedSink(std::unique_ptr<ILogSink> underlying,
            size_t bufferSize = 100,
            std::chrono::milliseconds flushInterval = std::chrono::milliseconds(1000));
    };

}