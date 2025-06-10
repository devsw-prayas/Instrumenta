#pragma once

#include <any>

#include "Instrumenta.h"
#include "LogUtils.h"
#include "Sinks.h"

namespace instrumenta {
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
        virtual void registerTag(std::string& tag, bool isEnabled, E_LogLevel minLevel, E_LogOutput outputDestinations) = 0;

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

    class INSTRUMENTA BaseLogger final : public ILogger, IRouterControl {
        static BaseLogger instance;
        std::string libraryName;
        std::shared_ptr<std::unordered_map<std::string, std::unique_ptr<ILogSink>>> sinks;
        std::unordered_map<std::string, TagConfig> tagConfigs;
        std::unordered_map<std::string, std::unordered_map<E_LogLevel, int>> logCounts;
        LogHistory logHistory;
        mutable std::mutex loggerMutex;
        std::unordered_map<std::string, std::unique_ptr<ISinkRouter>> routers;

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
        void registerTag(std::string& tag, bool isEnabled, E_LogLevel minLevel, E_LogOutput outputDestinations) override;

        // IRouterControl interface
        void clearRouters() override;
        ISinkRouter* getRouter(const std::string& routerTag) override;
        void registerRouter(const std::string& routerTag, std::unique_ptr<ISinkRouter> router) override;
        void removeRouter(const std::string& routerTag) override;

    private:
        static std::string getCurrentTimestamp();
        static std::vector<std::string> formatArgs(const std::vector<std::any>& args);
        TagConfig& getOrCreateTagConfig(const std::string& tag);
        void log(E_LogLevel level, const std::string& component, const std::string& subComponent,
            const std::string& message, const std::vector<std::any>& args = {},
            const std::string& tag = "") override;


    };
}
