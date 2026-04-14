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

		Orchestrator() noexcept;

		STRATUM_NODISCARD ILogger* findLogger(const char* p_Name) noexcept;

	public:
		Orchestrator(const Orchestrator&) = delete;
		Orchestrator& operator=(const Orchestrator&) = delete;
		Orchestrator(Orchestrator&&)      = delete;
		Orchestrator& operator=(Orchestrator&&) = delete;

		STRATUM_NODISCARD static Orchestrator& getInstance() noexcept;

		bool registerLogger(const char* p_Name, ILogger* po_Logger) noexcept;

		void log(
			Records::LogLevel           v_Level,
			const char*                 p_LoggerName,
			const char*                 p_Component,
			const char*                 p_Tag,
			const char*                 p_Message,
			const std::source_location& ro_Location = std::source_location::current(),
			bool                        v_ThreadTrace = false
		) noexcept;

		void trace(
			const char*                 p_LoggerName,
			const char*                 p_Tag,
			const Records::TracerEntry&  ro_Trace,
			bool                        v_ThreadTrace = false
		) noexcept;

		void exception(
			const char*                    p_LoggerName,
			const char*                    p_Tag,
			const Records::ExceptionEntry& ro_Exception,
			bool                           v_ThreadTrace = true
		) noexcept;

		void crash(
			const char*                 p_LoggerName,
			const char*                 p_Component,
			const char*                 p_Tag,
			const char*                 p_Message,
			const std::source_location& ro_Location = std::source_location::current()
		) noexcept;

		void flush() noexcept;
	};
}
