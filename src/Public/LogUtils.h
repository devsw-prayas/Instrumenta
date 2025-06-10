#pragma once

#include "Instrumenta.h"
#include <chrono>
#include <deque>
#include <mutex>

namespace instrumenta {
    enum class INSTRUMENTA E_LogOutput : uint8_t {
        NONE = 0,
        CONSOLE = 1,
        FILE = 1 << 1
    };

    inline E_LogOutput operator|(E_LogOutput lhs, E_LogOutput rhs) {
        return static_cast<E_LogOutput>(static_cast<uint32_t>(lhs) | static_cast<uint32_t>(rhs));
    }

    inline E_LogOutput operator&(E_LogOutput lhs, E_LogOutput rhs) {
        return static_cast<E_LogOutput>(static_cast<uint32_t>(lhs) & static_cast<uint32_t>(rhs));
    }

    enum class INSTRUMENTA E_LogLevel : uint8_t {
        DEBUG_ = 0,
        INFO_ = 1,
        WARNING_ = 2,
        ERROR_ = 3
    };

    class INSTRUMENTA LogEntry {
    public:
        std::string timestamp;
        E_LogLevel level;
        std::string libraryName;
        std::string component;
        std::string subComponent;
        std::string message;
        std::vector<std::string> formattedArgs;
        bool isColored;
        std::string tag;

        LogEntry(std::string ts, E_LogLevel lvl, const std::string& lib, const std::string& comp,
            const std::string& subComp, const std::string& msg, const std::vector<std::string>& args,
            bool colored, const std::string& logTag = "");
        
        [[nodiscard]] std::string toString() const;
        [[nodiscard]] std::string toColoredString() const;

    private:
        static std::string levelToString(E_LogLevel level, bool colored);
    };

    class INSTRUMENTA LogHistory {
        static constexpr size_t MAX_HISTORY_SIZE = 100;
        std::deque<LogEntry> history;
        mutable std::mutex historyMutex;

    public:
        void addLog(const LogEntry& entry);
        std::vector<std::string> getHistory() const;
        std::string getHistoryAsString() const;
        void clear();
        size_t size() const;
    };

    class INSTRUMENTA LoggedRuntimeError : public std::runtime_error {
        std::vector<std::string> history;

    public:
        LoggedRuntimeError(const std::string& message, const LogHistory& logHistory);
        [[nodiscard]] const std::vector<std::string>& getLogHistory() const;
        [[nodiscard]] std::string getFullMessage() const;
    };
}
