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

	static void INSTRUMENTA initializeInstrumentation() {
		auto& orchestrator = Instrumentation::getInstance();
		auto consoleSink = LogSinkFactory::createConsoleSink();
		auto fileSink = LogSinkFactory::createFileSink("default_instrumentation_log.txt");

		std::unique_ptr<TagRouter> consoleRouter = TagRouter::getInstance(CONSOLE_SINK_ROUTER_TAG);
		std::unique_ptr<TagRouter> fileRouter = TagRouter::getInstance(FILE_SINK_ROUTER_TAG);
		auto& defaultLogger = BaseLogger::getInstance();
        defaultLogger.registerSink(GENERIC_CONSOLE_OUTPUT_TAG, std::move(consoleSink));
		defaultLogger.registerSink(GENERIC_FILE_OUTPUT_TAG, std::move(fileSink));
        defaultLogger.registerRouter(CONSOLE_SINK_ROUTER_TAG, std::move(consoleRouter));
		defaultLogger.registerRouter(FILE_SINK_ROUTER_TAG, std::move(fileRouter));

        isInstrumentationInitialized = true;
	}

}