#pragma once
#ifndef INSTRUMENTA
#define INSTRUMENTA __declspec(dllexport)
#endif
#ifndef LOCATION
#define LOCATION "Line Number: " + std::to_string(__LINE__), "Function call: " + std::string(__FUNCSIG__), "File: " + std::string(__FILE__)
#endif

#include <string>
#include <fstream>
#include <unordered_map>
#include <any>
#include <vector>
#include <mutex>
#include <deque>
#include <memory>
#include <thread>
#include <condition_variable>
#include <atomic>

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

        LogEntry(std::string ts, E_LogLevel lvl, const std::string&& lib, const std::string&& comp,
            const std::string&& subComp, const std::string&& msg, const std::vector<std::string>& args,
            bool colored, const std::string&& logTag = "");

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

    // Core interfaces
    class INSTRUMENTA ILogSink {
    public:
        virtual ~ILogSink() = default;
        ILogSink(const ILogSink&) = delete;
        ILogSink& operator=(const ILogSink&) = delete;
        ILogSink(ILogSink&&) = delete;
        ILogSink& operator=(ILogSink&&) = delete;

        virtual void write(const LogEntry& entry) = 0;
        virtual void flush() = 0;
        virtual bool isEnabled() const = 0;
        virtual void setEnabled(bool enabled) = 0;
        virtual E_LogLevel getMinLevel() const = 0;
        virtual void setMinLevel(E_LogLevel level) = 0;

    protected:
        ILogSink() = default;
    };

    struct INSTRUMENTA TagConfig {
        bool enabled = true;
        E_LogLevel minLevel = E_LogLevel::DEBUG_;
        E_LogOutput outputDestinations = E_LogOutput::CONSOLE;
    };

    class INSTRUMENTA ILogger {
    public:
        virtual ~ILogger() = default;
        ILogger(const ILogger&) = delete;
        ILogger& operator=(const ILogger&) = delete;
        ILogger(ILogger&&) = delete;
        ILogger& operator=(ILogger&&) = delete;


        // Template method for variadic args
        template<typename... Args>
        void log(E_LogLevel level, const std::string& component, const std::string& subComponent,
            const std::string& message, const std::string& tag, Args&&... args) {
            std::vector<std::any> packedArgs;
            (packedArgs.emplace_back(std::forward<Args>(args)), ...);
            log(level, component, subComponent, message, packedArgs, tag);
        }

        // Sink management
        virtual void registerSink(const std::string& tag, std::unique_ptr<ILogSink> sink) = 0;
        virtual void removeSink(const std::string& tag) = 0;
        virtual ILogSink* getSink(const std::string& tag) = 0;

        // Tag configuration
        virtual void setTagConfig(const std::string& tag, const TagConfig& config) = 0;
        virtual TagConfig getTagConfig(const std::string& tag) const = 0;
        virtual void setEnabled(const std::string& tag, bool enable) = 0;
        [[nodiscard]] virtual bool isEnabled(const std::string& tag) const = 0;
        virtual void setMinLevel(const std::string& tag, E_LogLevel level) = 0;
        [[nodiscard]] virtual E_LogLevel getMinLevel(const std::string& tag) const = 0;
        virtual void setOutputDestinations(const std::string& tag, E_LogOutput destinations) = 0;

        // Statistics
        [[nodiscard]] virtual int getLogCount(const std::string& tag, E_LogLevel level) const = 0;
        [[nodiscard]] virtual int getTotalLogCount() const = 0;
        virtual void synchronousFlush(const std::string& tag = "") = 0;

        // History
        virtual const LogHistory& getHistory() const = 0;

    protected:
        ILogger() = default;
        // Core logging method
        virtual void log(
            E_LogLevel level,
            const std::string& component,
            const std::string& subComponent,
            const std::string& message,
            const std::vector<std::any>& args = {},
            const std::string& tag = ""
        ) = 0;
    };

    // Concrete implementations
    class INSTRUMENTA ConsoleSink final : public ILogSink {
        bool enabled;
        E_LogLevel minLevel;
        bool useColors;
        mutable std::mutex consoleMutex;

    public:
        explicit ConsoleSink(bool colors = true);
        ~ConsoleSink() override = default;

        void write(const LogEntry& entry) override;
        void flush() override;
        bool isEnabled() const override;
        void setEnabled(bool enabled) override;
        E_LogLevel getMinLevel() const override;
        void setMinLevel(E_LogLevel level) override;
        void setUseColors(bool colors);
    };

    class INSTRUMENTA FileSink final : public ILogSink {
        std::string filename;
        std::ofstream fileStream;
        bool enabled;
        E_LogLevel minLevel;
        mutable std::mutex fileMutex;
        std::vector<LogEntry> buffer;
        std::atomic<bool> autoFlush;

    public:
        explicit FileSink(const std::string& filename, bool autoFlush = true);
        ~FileSink() override;

        void write(const LogEntry& entry) override;
        void flush() override;
        bool isEnabled() const override;
        void setEnabled(bool enabled) override;
        E_LogLevel getMinLevel() const override;
        void setMinLevel(E_LogLevel level) override;
        void setAutoFlush(bool autoFlush);
        bool isOpen() const;
    };

    class INSTRUMENTA BufferedSink final : public ILogSink {
        std::unique_ptr<ILogSink> underlyingSink;
        std::vector<LogEntry> buffer;
        size_t bufferSize;
        mutable std::mutex bufferMutex;
        std::thread flushThread;
        std::condition_variable flushCV;
        std::atomic<bool> shouldStop;
        std::chrono::milliseconds flushInterval;

    public:
        BufferedSink(std::unique_ptr<ILogSink> sink, size_t bufferSize = 100,
            std::chrono::milliseconds flushInterval = std::chrono::milliseconds(1000));
        ~BufferedSink() override;

        void write(const LogEntry& entry) override;
        void flush() override;
        bool isEnabled() const override;
        void setEnabled(bool enabled) override;
        E_LogLevel getMinLevel() const override;
        void setMinLevel(E_LogLevel level) override;

    private:
        void flushWorker();
    };

    class INSTRUMENTA BaseLogger final : public ILogger {
        static BaseLogger instance;
        std::string libraryName;
        std::unordered_map<std::string, std::unique_ptr<ILogSink>> sinks;
        std::unordered_map<std::string, TagConfig> tagConfigs;
        std::unordered_map<std::string, std::unordered_map<E_LogLevel, int>> logCounts;
        LogHistory logHistory;
        mutable std::mutex loggerMutex;

        BaseLogger();

    public:
        static BaseLogger& getInstance();
        ~BaseLogger() override = default;

        void registerSink(const std::string& tag, std::unique_ptr<ILogSink> sink) override;
        void removeSink(const std::string& tag) override;
        ILogSink* getSink(const std::string& tag) override;

        void setTagConfig(const std::string& tag, const TagConfig& config) override;
        TagConfig getTagConfig(const std::string& tag) const override;
        void setEnabled(const std::string& tag, bool enable) override;
        bool isEnabled(const std::string& tag) const override;
        void setMinLevel(const std::string& tag, E_LogLevel level) override;
        E_LogLevel getMinLevel(const std::string& tag) const override;
        void setOutputDestinations(const std::string& tag, E_LogOutput destinations) override;

        int getLogCount(const std::string& tag, E_LogLevel level) const override;
        int getTotalLogCount() const override;
        void synchronousFlush(const std::string& tag = "") override;

        const LogHistory& getHistory() const override;
        void setLibraryName(const std::string& name);

    private:
        static std::string getCurrentTimestamp();
        static std::vector<std::string> formatArgs(const std::vector<std::any>& args);
        TagConfig& getOrCreateTagConfig(const std::string& tag);
        void log(E_LogLevel level, const std::string& component, const std::string& subComponent,
            const std::string& message, const std::vector<std::any>& args = {},
            const std::string& tag = "") override;
    };

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