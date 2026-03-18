#pragma once
#include "Stratum.h"
#include "Logger.h"
#include "StackTrace.h"

namespace Stratum::Logging {

    
    
    
    

    class STRATUM_API Orchestrator final {
    private:
        struct LoggerEntry {
            char     name[LOGGER_NAME_MAX];
            ILogger* logger;
        };

        LoggerEntry m_loggers[MAX_LOGGERS];
        size_t      m_loggerCount;

        Orchestrator() noexcept : m_loggers{}, m_loggerCount(0) {}

        STRATUM_NODISCARD ILogger* findLogger(const char* p_Name) noexcept {
            for (size_t i = 0; i < m_loggerCount; ++i)
                if (std::strncmp(m_loggers[i].name, p_Name, LOGGER_NAME_MAX) == 0)
                    return m_loggers[i].logger;
            return nullptr;
        }

    public:
        Orchestrator(const Orchestrator&)            = delete;
        Orchestrator& operator=(const Orchestrator&) = delete;
        Orchestrator(Orchestrator&&)                 = delete;
        Orchestrator& operator=(Orchestrator&&)      = delete;

        STRATUM_NODISCARD static Orchestrator& getInstance() noexcept {
            static Orchestrator s_instance;
            return s_instance;
        }

        

        bool registerLogger(const char* p_Name, ILogger* po_Logger) noexcept {
            if (!p_Name || !po_Logger || m_loggerCount >= MAX_LOGGERS) return false;
            std::strncpy(m_loggers[m_loggerCount].name, p_Name, LOGGER_NAME_MAX - 1);
            m_loggers[m_loggerCount].name[LOGGER_NAME_MAX - 1] = '\0';
            m_loggers[m_loggerCount].logger = po_Logger;
            ++m_loggerCount;
            return true;
        }

        void log(
            Records::LogLevel           v_Level,
            const char*                 p_LoggerName,
            const char*                 p_Component,
            const char*                 p_Tag,
            const char*                 p_Message,
            const std::source_location& ro_Location   = std::source_location::current(),
            bool                        v_ThreadTrace = false
        ) noexcept {
            ILogger* p_logger = findLogger(p_LoggerName);
            if (STRATUM_UNLIKELY(!p_logger)) return;

            Records::LogEntry entry{ v_Level, p_Component, p_Message, ro_Location };
            p_logger->onLog(p_Tag, entry);

            if (v_ThreadTrace)
                Tracing::this_thread::t_Trace.pushFrame(entry);
        }

        void trace(
            const char*                   p_LoggerName,
            const char*                   p_Tag,
            const Records::TracerEntry&   ro_Trace,
            bool                          v_ThreadTrace = false
        ) noexcept {
            ILogger* p_logger = findLogger(p_LoggerName);
            if (STRATUM_UNLIKELY(!p_logger)) return;

            p_logger->onLog(p_Tag, ro_Trace);

            if (v_ThreadTrace)
                Tracing::this_thread::t_Trace.pushFrame(ro_Trace);
        }

        void exception(
            const char*                      p_LoggerName,
            const char*                      p_Tag,
            const Records::ExceptionEntry&   ro_Exception,
            bool                             v_ThreadTrace = true
        ) noexcept {
            ILogger* p_logger = findLogger(p_LoggerName);
            if (STRATUM_UNLIKELY(!p_logger)) return;

            p_logger->onLog(p_Tag, ro_Exception);

            if (v_ThreadTrace)
                Tracing::this_thread::t_Trace.pushFrame(ro_Exception);
        }

        void crash(
            const char*                 p_LoggerName,
            const char*                 p_Component,
            const char*                 p_Tag,
            const char*                 p_Message,
            const std::source_location& ro_Location = std::source_location::current()
        ) noexcept {
            ILogger* p_logger = findLogger(p_LoggerName);
            if (STRATUM_UNLIKELY(!p_logger)) return;

            Records::LogEntry entry{ Records::LogLevel::Crash, p_Component, p_Message, ro_Location };
            p_logger->onLog(p_Tag, entry);
            Tracing::this_thread::t_Trace.pushFrame(entry);
        }

        void flush() noexcept {
            for (size_t i = 0; i < m_loggerCount; ++i)
                m_loggers[i].logger->flush();
        }
    };

}
