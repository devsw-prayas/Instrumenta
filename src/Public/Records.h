#pragma once
#include "Stratum.h"

namespace Stratum::Records {

    using Stratum::COMPONENT_NAME_MAX;
    using Stratum::MESSAGE_MAX;
    using Stratum::LABEL_MAX;
    using Stratum::LOGGER_NAME_MAX;
    using Stratum::TAG_MAX;
    using Stratum::MAX_ROUTES;
    using Stratum::MAX_LOGGERS;

    enum class LogLevel : uint8_t {
        Info, Warning, Debug, Error, Crash
    };

    enum class TracePolicy : uint8_t {
        AppendOnly, AutoFlushOnError, AutoFlushOnCrash, AutoFlushAlways
    };

    enum class TracerPrecision : uint8_t {
        Seconds, Milliseconds, Microseconds, Nanoseconds
    };

    struct STRATUM_API STRATUM_ALIGNAS(128) LogEntry final {
    private:
        LogLevel             m_level;
        char                 m_component[COMPONENT_NAME_MAX];
        char                 m_message[MESSAGE_MAX];
        uint64_t             m_timestamp;
        std::source_location m_location;
        uint64_t             m_threadId;

    public:
        LogEntry(
            LogLevel                    v_Level,
            const char*                 p_Component,
            const char*                 p_Message,
            const std::source_location& ro_Location = std::source_location::current()
        ) noexcept;

        STRATUM_NODISCARD LogLevel              getLevel()     const noexcept;
        STRATUM_NODISCARD const char*           getComponent() const noexcept;
        STRATUM_NODISCARD const char*           getMessage()   const noexcept;
        STRATUM_NODISCARD uint64_t              getTimestamp() const noexcept;
        STRATUM_NODISCARD std::source_location  getLocation()  const noexcept;
        STRATUM_NODISCARD uint64_t              getThreadId()  const noexcept;

        void setThreadId(uint64_t v_Id) noexcept;

        static const LogEntry s_invalid;
    };

    struct STRATUM_API STRATUM_ALIGNAS(128) ExceptionEntry final {
    private:
        char                 m_component[COMPONENT_NAME_MAX];
        char                 m_errorMessage[MESSAGE_MAX];
        uint64_t             m_timestamp;
        std::source_location m_location;
        uint64_t             m_threadId;

    public:
        ExceptionEntry(
            const char*                 p_Component,
            const char*                 p_ErrorMessage,
            const std::source_location& ro_Location = std::source_location::current()
        ) noexcept;

        STRATUM_NODISCARD const char*           getComponent()    const noexcept;
        STRATUM_NODISCARD const char*           getErrorMessage() const noexcept;
        STRATUM_NODISCARD uint64_t              getTimestamp()    const noexcept;
        STRATUM_NODISCARD std::source_location  getLocation()     const noexcept;
        STRATUM_NODISCARD uint64_t              getThreadId()     const noexcept;

        void setThreadId(uint64_t v_Id) noexcept;

        static const ExceptionEntry s_invalid;
    };

    struct STRATUM_API STRATUM_ALIGNAS(128) TracerEntry final {
    private:
        char                 m_component[COMPONENT_NAME_MAX];
        char                 m_label[LABEL_MAX];
        uint64_t             m_startTick;
        uint64_t             m_endTick;
        std::source_location m_location;
        uint64_t             m_threadId;
        bool                 m_started;
        bool                 m_complete;

    public:
        TracerEntry(
            const char*                 p_Component,
            const char*                 p_Label,
            const std::source_location& ro_Location = std::source_location::current()
        ) noexcept;

        STRATUM_NODISCARD const char*           getComponent() const noexcept;
        STRATUM_NODISCARD const char*           getLabel()     const noexcept;
        STRATUM_NODISCARD uint64_t              getStartTick() const noexcept;
        STRATUM_NODISCARD uint64_t              getEndTick()   const noexcept;
        STRATUM_NODISCARD std::source_location  getLocation()  const noexcept;
        STRATUM_NODISCARD uint64_t              getThreadId()  const noexcept;
        STRATUM_NODISCARD bool                  isStarted()    const noexcept;
        STRATUM_NODISCARD bool                  isComplete()   const noexcept;
        STRATUM_NODISCARD double                getDuration(TracerPrecision v_Precision) const noexcept;

        void setThreadId(uint64_t v_Id) noexcept;
        void start()                    noexcept;
        void end()                      noexcept;

        static const TracerEntry s_invalid;
    };

    struct STRATUM_API LoggerProfile final {
    private:
        char        m_name[LOGGER_NAME_MAX];
        void*       m_buffer;
        size_t      m_bufferSize;
        bool        m_enabled;
        TracePolicy m_tracePolicy;

    public:
        LoggerProfile(
            const char* p_Name,
            void*       p_Buffer,
            size_t      v_BufferSize
        ) noexcept;

        STRATUM_NODISCARD const char*  getName()       const noexcept;
        STRATUM_NODISCARD void*        getBuffer()     const noexcept;
        STRATUM_NODISCARD size_t       getBufferSize() const noexcept;
        STRATUM_NODISCARD bool         isEnabled()     const noexcept;
        STRATUM_NODISCARD TracePolicy  getPolicy()     const noexcept;

        void enable()                        noexcept;
        void disable()                       noexcept;
        bool setPolicy(TracePolicy v_Policy) noexcept;
    };

}
