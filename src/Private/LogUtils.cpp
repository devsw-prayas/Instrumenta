#include "LogUtils.h"

#include <mutex>

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
}
