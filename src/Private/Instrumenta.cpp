#include "instrumenta.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <algorithm>
#include <typeinfo>

#ifdef _WIN32
#include <windows.h>
#endif

namespace instrumenta {

    // LogEntry Implementation
    LogEntry::LogEntry(std::string ts, E_LogLevel lvl, const std::string&& lib, const std::string&& comp,
        const std::string&& subComp, const std::string&& msg, const std::vector<std::string>& args,
        bool colored, const std::string&& logTag)
        : timestamp(std::move(ts)), level(lvl), libraryName(lib), component(comp),
        subComponent(subComp), message(msg), formattedArgs(args), isColored(colored), tag(logTag) {
    }

    std::string LogEntry::toString() const {
        std::ostringstream oss;
        oss << "[" << timestamp << "] [" << levelToString(level, false) << "] ";
        if (!libraryName.empty()) oss << "[" << libraryName << "] ";
        if (!tag.empty()) oss << "[" << tag << "] ";
        oss << component << "::" << subComponent << " - " << message;

        if (!formattedArgs.empty()) {
            oss << " | Args: ";
            for (size_t i = 0; i < formattedArgs.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << formattedArgs[i];
            }
        }
        return oss.str();
    }

    std::string LogEntry::toColoredString() const {
        if (!isColored) return toString();

        std::ostringstream oss;
        oss << "[" << timestamp << "] [" << levelToString(level, true) << "] ";
        if (!libraryName.empty()) oss << "\033[36m[" << libraryName << "]\033[0m ";
        if (!tag.empty()) oss << "\033[35m[" << tag << "]\033[0m ";
        oss << "\033[32m" << component << "::" << subComponent << "\033[0m - " << message;

        if (!formattedArgs.empty()) {
            oss << " \033[33m| Args: ";
            for (size_t i = 0; i < formattedArgs.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << formattedArgs[i];
            }
            oss << "\033[0m";
        }
        return oss.str();
    }

    std::string LogEntry::levelToString(E_LogLevel level, bool colored) {
        if (colored) {
            switch (level) {
            case E_LogLevel::DEBUG_: return "\033[37mDEBUG\033[0m";
            case E_LogLevel::INFO_: return "\033[32mINFO\033[0m";
            case E_LogLevel::WARNING_: return "\033[33mWARN\033[0m";
            case E_LogLevel::ERROR_: return "\033[31mERROR\033[0m";
            }
        }
        else {
            switch (level) {
            case E_LogLevel::DEBUG_: return "DEBUG";
            case E_LogLevel::INFO_: return "INFO";
            case E_LogLevel::WARNING_: return "WARN";
            case E_LogLevel::ERROR_: return "ERROR";
            }
        }
        return "\033[37mUNKNOWN\033[0m";
    }

    // LogHistory Implementation
    void LogHistory::addLog(const LogEntry& entry) {
        std::lock_guard<std::mutex> lock(historyMutex);
        history.push_back(entry);
        if (history.size() > MAX_HISTORY_SIZE) {
            history.pop_front();
        }
    }

    std::vector<std::string> LogHistory::getHistory() const {
        std::lock_guard<std::mutex> lock(historyMutex);
        std::vector<std::string> result;
        result.reserve(history.size());
        for (const auto& entry : history) {
            result.push_back(entry.toString());
        }
        return result;
    }

    std::string LogHistory::getHistoryAsString() const {
        std::lock_guard<std::mutex> lock(historyMutex);
        std::ostringstream oss;
        for (const auto& entry : history) {
            oss << entry.toString() << "\n";
        }
        return oss.str();
    }

    void LogHistory::clear() {
        std::lock_guard<std::mutex> lock(historyMutex);
        history.clear();
    }

    size_t LogHistory::size() const {
        std::lock_guard<std::mutex> lock(historyMutex);
        return history.size();
    }

    // LoggedRuntimeError Implementation
    LoggedRuntimeError::LoggedRuntimeError(const std::string& message, const LogHistory& logHistory)
        : std::runtime_error(message), history(logHistory.getHistory()) {}

    const std::vector<std::string>& LoggedRuntimeError::getLogHistory() const {
        return history;
    }

    std::string LoggedRuntimeError::getFullMessage() const {
        std::ostringstream oss;
        oss << what() << "\n\nLog History:\n";
        for (const auto& entry : history) {
            oss << entry << "\n";
        }
        return oss.str();
    }

    // ConsoleSink Implementation
    ConsoleSink::ConsoleSink(bool colors) : enabled(true), minLevel(E_LogLevel::DEBUG_), useColors(colors) {}

    void ConsoleSink::write(const LogEntry& entry) {
        if (!enabled || entry.level < minLevel) return;

        std::lock_guard<std::mutex> lock(consoleMutex);
        if (useColors) {
            std::cout << entry.toColoredString() << "\n";
        }
        else {
            std::cout << entry.toString() << "\n";
        }
    }

    void ConsoleSink::flush() {
        std::lock_guard<std::mutex> lock(consoleMutex);
        std::cout.flush();
    }

    bool ConsoleSink::isEnabled() const {
        return enabled;
    }

    void ConsoleSink::setEnabled(bool enable) {
        enabled = enable;
    }

    E_LogLevel ConsoleSink::getMinLevel() const {
        return minLevel;
    }

    void ConsoleSink::setMinLevel(E_LogLevel level) {
        minLevel = level;
    }

    void ConsoleSink::setUseColors(bool colors) {
        useColors = colors;
    }

    // FileSink Implementation
    FileSink::FileSink(const std::string& filename, bool autoFlush)
        : filename(filename), enabled(true), minLevel(E_LogLevel::DEBUG_), autoFlush(autoFlush) {
        fileStream.open(filename, std::ios::out | std::ios::app);
        if (!fileStream.is_open()) {
            throw std::runtime_error("Failed to open log file: " + filename);
        }
    }

    FileSink::~FileSink() {
        if (fileStream.is_open()) {
            std::lock_guard<std::mutex> lock(fileMutex);
            if (!fileStream.is_open()) return;

            for (const auto& entry : buffer) {
                fileStream << entry.toString() << "\n";
            }
            buffer.clear();
            fileStream.flush();
            fileStream.close();
        }
    }

    void FileSink::write(const LogEntry& entry) {
        if (!enabled || entry.level < minLevel || !fileStream.is_open()) return;

        std::lock_guard<std::mutex> lock(fileMutex);
        if (autoFlush.load()) {
            fileStream << entry.toString() << "\n";
            fileStream.flush();
        }
        else {
            buffer.push_back(entry);
        }
    }

    void FileSink::flush() {
        std::lock_guard<std::mutex> lock(fileMutex);
        if (!fileStream.is_open()) return;

        for (const auto& entry : buffer) {
            fileStream << entry.toString() << "\n";
        }
        buffer.clear();
        fileStream.flush();
    }

    bool FileSink::isEnabled() const {
        return enabled;
    }

    void FileSink::setEnabled(bool enable) {
        enabled = enable;
    }

    E_LogLevel FileSink::getMinLevel() const {
        return minLevel;
    }

    void FileSink::setMinLevel(E_LogLevel level) {
        minLevel = level;
    }

    void FileSink::setAutoFlush(bool enable) {
        autoFlush.store(enable);
    }

    bool FileSink::isOpen() const {
        return fileStream.is_open();
    }

    // BufferedSink Implementation
    BufferedSink::BufferedSink(std::unique_ptr<ILogSink> sink, size_t bufferSize,
        std::chrono::milliseconds flushInterval)
        : underlyingSink(std::move(sink)), bufferSize(bufferSize), shouldStop(false),
        flushInterval(flushInterval) {
        buffer.reserve(bufferSize);
        flushThread = std::thread(&BufferedSink::flushWorker, this);
    }

    BufferedSink::~BufferedSink() {
        shouldStop.store(true);
        flushCV.notify_all();
        if (flushThread.joinable()) {
            flushThread.join();
        }
        std::unique_lock<std::mutex> lock(bufferMutex);
        if (!underlyingSink || buffer.empty()) return;

        for (const auto& entry : buffer) {
            underlyingSink->write(entry);
        }
        buffer.clear();
        underlyingSink->flush();
    }

    void BufferedSink::write(const LogEntry& entry) {
        if (!underlyingSink || !underlyingSink->isEnabled() ||
            entry.level < underlyingSink->getMinLevel()) return;

        std::lock_guard<std::mutex> lock(bufferMutex);
        buffer.push_back(entry);
        if (buffer.size() >= bufferSize) {
            flushCV.notify_one();
        }
    }

    void BufferedSink::flush() {
        std::unique_lock<std::mutex> lock(bufferMutex);
        if (!underlyingSink || buffer.empty()) return;

        for (const auto& entry : buffer) {
            underlyingSink->write(entry);
        }
        buffer.clear();
        underlyingSink->flush();
    }

    bool BufferedSink::isEnabled() const {
        return underlyingSink ? underlyingSink->isEnabled() : false;
    }

    void BufferedSink::setEnabled(bool enable) {
        if (underlyingSink) underlyingSink->setEnabled(enable);
    }

    E_LogLevel BufferedSink::getMinLevel() const {
        return underlyingSink ? underlyingSink->getMinLevel() : E_LogLevel::DEBUG_;
    }

    void BufferedSink::setMinLevel(E_LogLevel level) {
        if (underlyingSink) underlyingSink->setMinLevel(level);
    }

    void BufferedSink::flushWorker() {
        while (!shouldStop.load()) {
            std::unique_lock<std::mutex> lock(bufferMutex);
            flushCV.wait_for(lock, flushInterval, [this] {
                return shouldStop.load() || !buffer.empty();
                });

            if (!buffer.empty()) {
                for (const auto& entry : buffer) {
                    if (underlyingSink) underlyingSink->write(entry);
                }
                buffer.clear();
                if (underlyingSink) underlyingSink->flush();
            }
        }
    }

    // BaseLogger Implementation
    BaseLogger BaseLogger::instance;

    BaseLogger::BaseLogger() : libraryName("instrumenta") {}

    BaseLogger& BaseLogger::getInstance() {
        return instance;
    }

    void BaseLogger::log(E_LogLevel level, const std::string& component, const std::string& subComponent,
        const std::string& message, const std::vector<std::any>& args, const std::string& tag) {
        std::lock_guard<std::mutex> lock(loggerMutex);

        const std::string& activeTag = tag.empty() ? "default" : tag;
        TagConfig& config = getOrCreateTagConfig(activeTag);

        if (!config.enabled || level < config.minLevel) return;

        std::string timestamp = getCurrentTimestamp();
        std::vector<std::string> formattedArgs = formatArgs(args);
        std::string tempTag = activeTag;
        LogEntry entry(timestamp, level, std::move(libraryName), std::move(component), 
            std::move(subComponent), std::move(message), formattedArgs, true, std::move(activeTag));

        logHistory.addLog(entry);
        logCounts[tempTag][level]++;

        // Write to appropriate sinks based on output destinations
        if ((config.outputDestinations & E_LogOutput::CONSOLE) != E_LogOutput::NONE) {
            auto consoleSink = sinks.find("console");
            if (consoleSink != sinks.end() && consoleSink->second) {
                consoleSink->second->write(entry);
            }
        }

        if ((config.outputDestinations & E_LogOutput::FILE) != E_LogOutput::NONE) {
            auto fileSink = sinks.find("file");
            if (fileSink != sinks.end() && fileSink->second) {
                fileSink->second->write(entry);
            }
        }

        // Write to tag-specific sink if it exists
        auto tagSink = sinks.find(tempTag);
        if (tagSink != sinks.end() && tagSink->second) {
            tagSink->second->write(entry);
        }
    }

    void BaseLogger::registerSink(const std::string& tag, std::unique_ptr<ILogSink> sink) {
        std::lock_guard<std::mutex> lock(loggerMutex);
        sinks[tag] = std::move(sink);
    }

    void BaseLogger::removeSink(const std::string& tag) {
        std::lock_guard<std::mutex> lock(loggerMutex);
        sinks.erase(tag);
    }

    ILogSink* BaseLogger::getSink(const std::string& tag) {
        std::lock_guard<std::mutex> lock(loggerMutex);
        auto it = sinks.find(tag);
        return (it != sinks.end()) ? it->second.get() : nullptr;
    }

    void BaseLogger::setTagConfig(const std::string& tag, const TagConfig& config) {
        std::lock_guard<std::mutex> lock(loggerMutex);
        tagConfigs[tag] = config;
    }

    TagConfig BaseLogger::getTagConfig(const std::string& tag) const {
        std::lock_guard<std::mutex> lock(loggerMutex);
        auto it = tagConfigs.find(tag);
        return (it != tagConfigs.end()) ? it->second : TagConfig{};
    }

    void BaseLogger::setEnabled(const std::string& tag, bool enable) {
        std::lock_guard<std::mutex> lock(loggerMutex);
        getOrCreateTagConfig(tag).enabled = enable;
    }

    bool BaseLogger::isEnabled(const std::string& tag) const {
        std::lock_guard<std::mutex> lock(loggerMutex);
        auto it = tagConfigs.find(tag);
        return (it != tagConfigs.end()) ? it->second.enabled : true;
    }

    void BaseLogger::setMinLevel(const std::string& tag, E_LogLevel level) {
        std::lock_guard<std::mutex> lock(loggerMutex);
        getOrCreateTagConfig(tag).minLevel = level;
    }

    E_LogLevel BaseLogger::getMinLevel(const std::string& tag) const {
        std::lock_guard<std::mutex> lock(loggerMutex);
        auto it = tagConfigs.find(tag);
        return (it != tagConfigs.end()) ? it->second.minLevel : E_LogLevel::DEBUG_;
    }

    void BaseLogger::setOutputDestinations(const std::string& tag, E_LogOutput destinations) {
        std::lock_guard<std::mutex> lock(loggerMutex);
        getOrCreateTagConfig(tag).outputDestinations = destinations;
    }

    int BaseLogger::getLogCount(const std::string& tag, E_LogLevel level) const {
        std::lock_guard<std::mutex> lock(loggerMutex);
        auto tagIt = logCounts.find(tag);
        if (tagIt != logCounts.end()) {
            auto levelIt = tagIt->second.find(level);
            if (levelIt != tagIt->second.end()) {
                return levelIt->second;
            }
        }
        return 0;
    }

    int BaseLogger::getTotalLogCount() const {
        std::lock_guard<std::mutex> lock(loggerMutex);
        int total = 0;
        for (const auto& tagPair : logCounts) {
            for (const auto& levelPair : tagPair.second) {
                total += levelPair.second;
            }
        }
        return total;
    }

    void BaseLogger::synchronousFlush(const std::string& tag) {
        std::lock_guard<std::mutex> lock(loggerMutex);
        if (tag.empty()) {
            for (auto& sinkPair : sinks) {
                if (sinkPair.second) sinkPair.second->flush();
            }
        }
        else {
            auto it = sinks.find(tag);
            if (it != sinks.end() && it->second) {
                it->second->flush();
            }
        }
    }

    const LogHistory& BaseLogger::getHistory() const {
        return logHistory;
    }

    void BaseLogger::setLibraryName(const std::string& name) {
        std::lock_guard<std::mutex> lock(loggerMutex);
        libraryName = name;
    }

    std::string BaseLogger::getCurrentTimestamp() {
        try {
            auto now = std::chrono::system_clock::now();
            auto localTime = std::chrono::zoned_time{ std::chrono::current_zone(), now };
            return std::format("{:%Y-%m-%d %H:%M:%S}", localTime);
        }
        catch (const std::exception& e) {
            return "[ERROR_: Timestamp fail: " + std::string(e.what()) + "]";
        }
    }

    std::vector<std::string> BaseLogger::formatArgs(const std::vector<std::any>& args) {
        std::vector<std::string> formatted;
        formatted.reserve(args.size());

        for (const auto& arg : args) {
            try {
                if (arg.type() == typeid(int)) {
                    formatted.push_back(std::to_string(std::any_cast<int>(arg)));
                }
                else if (arg.type() == typeid(double)) {
                    formatted.push_back(std::to_string(std::any_cast<double>(arg)));
                }
                else if (arg.type() == typeid(float)) {
                    formatted.push_back(std::to_string(std::any_cast<float>(arg)));
                }
                else if (arg.type() == typeid(std::string)) {
                    formatted.push_back(std::any_cast<std::string>(arg));
                }
                else if (arg.type() == typeid(const char*)) {
                    formatted.emplace_back(std::any_cast<const char*>(arg));
                }
                else if (arg.type() == typeid(bool)) {
                    formatted.emplace_back(std::any_cast<bool>(arg) ? "true" : "false");
                }
                else {
                    formatted.emplace_back("<unknown_type>");
                }
            }
            catch (const std::bad_any_cast&) {
                formatted.emplace_back("<cast_error>");
            }
        }
        return formatted;
    }

    TagConfig& BaseLogger::getOrCreateTagConfig(const std::string& tag) {
        auto it = tagConfigs.find(tag);
        if (it == tagConfigs.end()) {
            tagConfigs[tag] = TagConfig{};
            return tagConfigs[tag];
        }
        return it->second;
    }

    // Instrumentation Implementation
    Instrumentation Instrumentation::instance;

    Instrumentation& Instrumentation::getInstance() {
        return instance;
    }

    void Instrumentation::registerLogger(const std::string& name, std::unique_ptr<ILogger> logger) {
        std::lock_guard<std::mutex> lock(registryMutex);
        loggerRegistry[name] = std::move(logger);
    }

    ILogger* Instrumentation::getLogger(const std::string& name) {
        std::lock_guard<std::mutex> lock(registryMutex);
        auto it = loggerRegistry.find(name);
        return (it != loggerRegistry.end()) ? it->second.get() : nullptr;
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