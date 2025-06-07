#pragma once
#include <mutex>
#include <unordered_map>
#include <fstream>

#include "Instrumenta.h"
#include "LogUtils.h"

namespace instrumenta {
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

    class INSTRUMENTA ISinkRouter {
    public:
        virtual ~ISinkRouter() = default;
        ISinkRouter(const ISinkRouter&) = delete;
        ISinkRouter& operator=(const ISinkRouter&) = delete;
        ISinkRouter(ISinkRouter&&) = delete;
        ISinkRouter& operator=(ISinkRouter&&) = delete;
        virtual bool route(const LogEntry& entry, std::unique_ptr<std::unordered_map<std::string, std::unique_ptr<ILogSink*> >>) = 0;

    protected:
        ISinkRouter() = default;
    };

    class INSTRUMENTA IRouterControl {
    public:
        virtual ~IRouterControl() = default;
        IRouterControl(const IRouterControl&) = delete;
        IRouterControl& operator=(const IRouterControl&) = delete;
        IRouterControl(IRouterControl&&) = delete;
        IRouterControl& operator=(IRouterControl&&) = delete;
        virtual void registerRouter(const std::string& routerTag, std::unique_ptr<ISinkRouter> router) = 0;
        virtual void removeRouter(const std::string& routerTag) = 0;
        virtual ISinkRouter* getRouter(const std::string& routerTag) = 0;
        virtual void clearRouters() = 0;

    protected:
        IRouterControl() = default;
    };

    struct INSTRUMENTA TagConfig {
        bool enabled = true;
        E_LogLevel minLevel = E_LogLevel::DEBUG_;
        E_LogOutput outputDestinations = E_LogOutput::CONSOLE;
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

    class INSTRUMENTA DefaultRouter final : public ISinkRouter {
        static DefaultRouter instance;
        std::vector<std::string> tags;
    public:
        DefaultRouter& getInstance();
        ~DefaultRouter() override = default;
        bool route(const LogEntry& entry, std::unique_ptr<std::unordered_map<std::string, std::unique_ptr<ILogSink*>>>) override;
    };
}
