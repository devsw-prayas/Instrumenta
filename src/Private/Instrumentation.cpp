	#include "Instrumentation.h"

namespace instrumenta {

    // Instrumentation Implementation
    Instrumentation Instrumentation::instance;

    Instrumentation& Instrumentation::getInstance() {
        return instance;
    }

    void Instrumentation::registerLogger(const std::string& name, ILogger* logger) {
        std::lock_guard<std::mutex> lock(registryMutex);
        loggerRegistry[name] = logger;
    }

    ILogger* Instrumentation::getLogger(const std::string& name) {
        std::lock_guard<std::mutex> lock(registryMutex);
        auto it = loggerRegistry.find(name);
        return (it != loggerRegistry.end()) ? it->second : nullptr;
    }

    void Instrumentation::removeLogger(const std::string& name) {
        std::lock_guard<std::mutex> lock(registryMutex);
        loggerRegistry.erase(name);
    }

    std::vector<std::string> Instrumentation::getRegisteredLoggers() const {
        std::lock_guard<std::mutex> lock(registryMutex);
        std::vector<std::string> names;
        names.reserve(loggerRegistry.size());
        for (const auto& pair : loggerRegistry) {
            names.push_back(pair.first);
        }
        return names;
    }

    void Instrumentation::flushAll() const {
        std::lock_guard<std::mutex> lock(registryMutex);
        for (auto& pair : loggerRegistry) {
            if (pair.second) {
                pair.second->synchronousFlush();
            }
        }
        BaseLogger::getInstance().synchronousFlush();
    }

    void Instrumentation::shutdown() {
        flushAll();
        std::lock_guard<std::mutex> lock(registryMutex);
        loggerRegistry.clear();
    }

    // LogSinkFactory Implementation
    std::unique_ptr<ILogSink> LogSinkFactory::createConsoleSink(bool useColors) {
        return std::make_unique<ConsoleSink>(useColors);
    }

    std::unique_ptr<ILogSink> LogSinkFactory::createFileSink(const std::string& filename, bool autoFlush) {
        return std::make_unique<FileSink>(filename, autoFlush);
    }

    std::unique_ptr<ILogSink> LogSinkFactory::createBufferedSink(std::unique_ptr<ILogSink> underlying,
        size_t bufferSize, std::chrono::milliseconds flushInterval) {
        return std::make_unique<BufferedSink>(std::move(underlying), bufferSize, flushInterval);
    }

}
