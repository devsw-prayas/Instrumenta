#include "../Public/Stratum.h"
#include "../Public/Records.h"

namespace Stratum::Records {

    

    static STRATUM_FORCEINLINE uint64_t currentTick() noexcept {
        return static_cast<uint64_t>(
            std::chrono::steady_clock::now().time_since_epoch().count()
        );
    }

    static STRATUM_FORCEINLINE void copyStr(
        char*       p_Dst,
        const char* p_Src,
        size_t      v_Max
    ) noexcept {
        if (STRATUM_LIKELY(p_Src != nullptr)) {
            std::strncpy(p_Dst, p_Src, v_Max - 1);
            p_Dst[v_Max - 1] = '\0';
        } else {
            p_Dst[0] = '\0';
        }
    }

    

    const LogEntry LogEntry::s_invalid{
        LogLevel::Crash, "NONE", "Invalid LogEntry"
    };

    LogEntry::LogEntry(
        LogLevel                    v_Level,
        const char*                 p_Component,
        const char*                 p_Message,
        const std::source_location& ro_Location
    ) noexcept
        : m_level(v_Level)
        , m_timestamp(currentTick())
        , m_location(ro_Location)
        , m_threadId(0) {
        copyStr(m_component, p_Component, COMPONENT_NAME_MAX);
        copyStr(m_message,   p_Message,   MESSAGE_MAX);
    }

    LogLevel             LogEntry::getLevel()     const noexcept { return m_level; }
    const char*          LogEntry::getComponent() const noexcept { return m_component; }
    const char*          LogEntry::getMessage()   const noexcept { return m_message; }
    uint64_t             LogEntry::getTimestamp() const noexcept { return m_timestamp; }
    std::source_location LogEntry::getLocation()  const noexcept { return m_location; }
    uint64_t             LogEntry::getThreadId()  const noexcept { return m_threadId; }

    void LogEntry::setThreadId(uint64_t v_Id) noexcept { m_threadId = v_Id; }

    

    const ExceptionEntry ExceptionEntry::s_invalid{
        "NONE", "Invalid ExceptionEntry"
    };

    ExceptionEntry::ExceptionEntry(
        const char*                 p_Component,
        const char*                 p_ErrorMessage,
        const std::source_location& ro_Location
    ) noexcept
        : m_timestamp(currentTick())
        , m_location(ro_Location)
        , m_threadId(0) {
        copyStr(m_component,    p_Component,    COMPONENT_NAME_MAX);
        copyStr(m_errorMessage, p_ErrorMessage, MESSAGE_MAX);
    }

    const char*          ExceptionEntry::getComponent()    const noexcept { return m_component; }
    const char*          ExceptionEntry::getErrorMessage() const noexcept { return m_errorMessage; }
    uint64_t             ExceptionEntry::getTimestamp()    const noexcept { return m_timestamp; }
    std::source_location ExceptionEntry::getLocation()     const noexcept { return m_location; }
    uint64_t             ExceptionEntry::getThreadId()     const noexcept { return m_threadId; }

    void ExceptionEntry::setThreadId(uint64_t v_Id) noexcept { m_threadId = v_Id; }

    

    const TracerEntry TracerEntry::s_invalid{
        "NONE", "Invalid TracerEntry"
    };

    TracerEntry::TracerEntry(
        const char*                 p_Component,
        const char*                 p_Label,
        const std::source_location& ro_Location
    ) noexcept
        : m_startTick(0)
        , m_endTick(0)
        , m_location(ro_Location)
        , m_threadId(0)
        , m_started(false)
        , m_complete(false) {
        copyStr(m_component, p_Component, COMPONENT_NAME_MAX);
        copyStr(m_label,     p_Label,     LABEL_MAX);
    }

    const char*          TracerEntry::getComponent() const noexcept { return m_component; }
    const char*          TracerEntry::getLabel()     const noexcept { return m_label; }
    uint64_t             TracerEntry::getStartTick() const noexcept { return m_startTick; }
    uint64_t             TracerEntry::getEndTick()   const noexcept { return m_endTick; }
    std::source_location TracerEntry::getLocation()  const noexcept { return m_location; }
    uint64_t             TracerEntry::getThreadId()  const noexcept { return m_threadId; }
    bool                 TracerEntry::isStarted()    const noexcept { return m_started; }
    bool                 TracerEntry::isComplete()   const noexcept { return m_complete; }

    void TracerEntry::setThreadId(uint64_t v_Id) noexcept { m_threadId = v_Id; }

    void TracerEntry::start() noexcept {
        if (STRATUM_UNLIKELY(m_started)) return;
        m_startTick = currentTick();
        m_started   = true;
    }

    void TracerEntry::end() noexcept {
        if (STRATUM_UNLIKELY(!m_started || m_complete)) return;
        m_endTick  = currentTick();
        m_complete = true;
    }

    double TracerEntry::getDuration(TracerPrecision v_Precision) const noexcept {
        if (STRATUM_UNLIKELY(!m_complete)) return 0.0;

        const uint64_t deltaNs = m_endTick - m_startTick;
        switch (v_Precision) {
            case TracerPrecision::Nanoseconds:  return static_cast<double>(deltaNs);
            case TracerPrecision::Microseconds: return static_cast<double>(deltaNs) * 1e-3;
            case TracerPrecision::Milliseconds: return static_cast<double>(deltaNs) * 1e-6;
            case TracerPrecision::Seconds:      return static_cast<double>(deltaNs) * 1e-9;
        }
        STRATUM_UNREACHABLE();
        return 0.0;
    }

    

    LoggerProfile::LoggerProfile(
        const char* p_Name,
        void*       p_Buffer,
        size_t      v_BufferSize
    ) noexcept
        : m_buffer(p_Buffer)
        , m_bufferSize(v_BufferSize)
        , m_enabled(true)
        , m_tracePolicy(TracePolicy::AutoFlushAlways) {
        copyStr(m_name, p_Name, LOGGER_NAME_MAX);
    }

    const char*  LoggerProfile::getName()       const noexcept { return m_name; }
    void*        LoggerProfile::getBuffer()     const noexcept { return m_buffer; }
    size_t       LoggerProfile::getBufferSize() const noexcept { return m_bufferSize; }
    bool         LoggerProfile::isEnabled()     const noexcept { return m_enabled; }
    TracePolicy  LoggerProfile::getPolicy()     const noexcept { return m_tracePolicy; }

    void LoggerProfile::enable()  noexcept { m_enabled = true; }
    void LoggerProfile::disable() noexcept { m_enabled = false; }

    bool LoggerProfile::setPolicy(TracePolicy v_Policy) noexcept {
        if (v_Policy == m_tracePolicy) return false;
        m_tracePolicy = v_Policy;
        return true;
    }

}
