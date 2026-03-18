#include "../Public/Stratum.h"
#include "../Public/Logger.h"

namespace Stratum::Logging {

    Logger::Logger(ILogger* po_Logger) noexcept
        : m_logger(po_Logger) {}

    bool Logger::valid() const noexcept {
        return m_logger != nullptr;
    }

    void Logger::log(const char* p_Tag, const Records::LogEntry& ro_Entry) const noexcept {
        STRATUM_ASSERT(m_logger != nullptr);
        m_logger->onLog(p_Tag, ro_Entry);
    }

    void Logger::log(const char* p_Tag, const Records::ExceptionEntry& ro_Entry) const noexcept {
        STRATUM_ASSERT(m_logger != nullptr);
        m_logger->onLog(p_Tag, ro_Entry);
    }

    void Logger::log(const char* p_Tag, const Records::TracerEntry& ro_Entry) const noexcept {
        STRATUM_ASSERT(m_logger != nullptr);
        m_logger->onLog(p_Tag, ro_Entry);
    }

    void Logger::flush() const noexcept {
        STRATUM_ASSERT(m_logger != nullptr);
        m_logger->flush();
    }

    void Logger::addRoute(const char* p_Tag, ISinkRouter* po_Router) const noexcept {
        STRATUM_ASSERT(m_logger != nullptr);
        m_logger->addRoute(p_Tag, po_Router);
    }

}
