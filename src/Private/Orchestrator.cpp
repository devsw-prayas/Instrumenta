#include "Orchestrator.h"

namespace Stratum::Logging {

	Orchestrator::Orchestrator() noexcept : m_loggers{}, m_loggerCount(0) {}

	Orchestrator& Orchestrator::getInstance() noexcept {
		static Orchestrator s_instance;
		return s_instance;
	}

	ILogger* Orchestrator::findLogger(const char* p_Name) noexcept {
		for (size_t i = 0; i < m_loggerCount; ++i)
			if (std::strncmp(m_loggers[i].name, p_Name, LOGGER_NAME_MAX) == 0)
				return m_loggers[i].logger;
		return nullptr;
	}

	bool Orchestrator::registerLogger(const char* p_Name, ILogger* po_Logger) noexcept {
		if (!p_Name || !po_Logger || m_loggerCount >= MAX_LOGGERS) return false;
		std::strncpy(m_loggers[m_loggerCount].name, p_Name, LOGGER_NAME_MAX - 1);
		m_loggers[m_loggerCount].name[LOGGER_NAME_MAX - 1] = '\0';
		m_loggers[m_loggerCount].logger = po_Logger;
		++m_loggerCount;
		return true;
	}

	void Orchestrator::log(
		Records::LogLevel v_Level,
		const char* p_LoggerName,
		const char* p_Component,
		const char* p_Tag,
		const char* p_Message,
		const std::source_location& ro_Location,
		bool v_ThreadTrace) noexcept {
		
		ILogger* p_logger = findLogger(p_LoggerName);
		if (STRATUM_UNLIKELY(!p_logger)) return;

		Records::LogEntry entry{ v_Level, p_Component, p_Message, ro_Location };
		p_logger->onLog(p_Tag, entry);

		if (v_ThreadTrace)
			Tracing::this_thread::t_Trace.pushFrame(entry);
	}

	void Orchestrator::trace(
		const char* p_LoggerName,
		const char* p_Tag,
		const Records::TracerEntry& ro_Trace,
		bool v_ThreadTrace) noexcept {
		
		ILogger* p_logger = findLogger(p_LoggerName);
		if (STRATUM_UNLIKELY(!p_logger)) return;

		p_logger->onLog(p_Tag, ro_Trace);

		if (v_ThreadTrace)
			Tracing::this_thread::t_Trace.pushFrame(ro_Trace);
	}

	void Orchestrator::exception(
		const char* p_LoggerName,
		const char* p_Tag,
		const Records::ExceptionEntry& ro_Exception,
		bool v_ThreadTrace) noexcept {
		
		ILogger* p_logger = findLogger(p_LoggerName);
		if (STRATUM_UNLIKELY(!p_logger)) return;

		p_logger->onLog(p_Tag, ro_Exception);

		if (v_ThreadTrace)
			Tracing::this_thread::t_Trace.pushFrame(ro_Exception);
	}

	void Orchestrator::crash(
		const char* p_LoggerName,
		const char* p_Component,
		const char* p_Tag,
		const char* p_Message,
		const std::source_location& ro_Location) noexcept {
		
		ILogger* p_logger = findLogger(p_LoggerName);
		if (STRATUM_UNLIKELY(!p_logger)) return;

		Records::LogEntry entry{ Records::LogLevel::Crash, p_Component, p_Message, ro_Location };
		p_logger->onLog(p_Tag, entry);
		Tracing::this_thread::t_Trace.pushFrame(entry);
	}

	void Orchestrator::flush() noexcept {
		for (size_t i = 0; i < m_loggerCount; ++i)
			m_loggers[i].logger->flush();
	}

} // namespace Stratum::Logging
