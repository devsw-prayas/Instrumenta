#pragma once
#include "Stratum.h"
#include "LogRoutes.h"
#include "StackTrace.h"

namespace Stratum::Logging {
	class STRATUM Orchestrator final {
		std::unordered_map<std::string, Logger*> m_Loggers;

	public:
		static Orchestrator& getInstance() noexcept {
			static Orchestrator s_Instance;
			return s_Instance;
		}

		void registerLogger(Logger* p_Logger, const char* p_Code) noexcept {
			if (!p_Logger || !p_Code) return;
			m_Loggers.emplace(p_Code, p_Logger);
		}

		void log(Tracing::LogLevel v_Level,
			const char* p_Logger, const char* p_Component,
			const char* p_Tag,
			std::string&& u_Message,
			const std::source_location& v_Location = std::source_location::current(),
			bool v_ThreadTrace = false) noexcept {
			auto it = m_Loggers.find(p_Logger);												  
			if (it == m_Loggers.end()) return;

			Logger* pLogger = it->second;
			if (!pLogger) return;

			Tracing::LogEntry entry{ v_Level, p_Component, std::move(u_Message), v_Location };
			pLogger->log(p_Tag, entry);

			if (v_ThreadTrace)
				Tracing::this_tracer::trace.pushFrame(entry);
		}

		void trace(const char* p_Logger,
				   const char* p_Tag,
				   const Tracing::TracingProfile& ro_Trace,
				   bool v_ThreadTrace = false) noexcept {
			auto it = m_Loggers.find(p_Logger);
			if (it == m_Loggers.end()) return;

			Logger* pLogger = it->second;
			if (!pLogger) return;

			pLogger->log(p_Tag, ro_Trace);

			if (v_ThreadTrace)
				Tracing::this_tracer::trace.pushFrame(ro_Trace);
		}

		void exception(const char* p_Logger,
					   const char* p_Tag,
					   const Tracing::ExceptionProfile& ro_Exception,
					   bool v_ThreadTrace = true) noexcept {
			auto it = m_Loggers.find(p_Logger);
			if (it == m_Loggers.end()) return;

			Logger* pLogger = it->second;
			if (!pLogger) return;

			pLogger->log(p_Tag, ro_Exception);

			if (v_ThreadTrace)
				Tracing::this_tracer::trace.pushFrame(ro_Exception);
		}

		void crash(const char* p_Logger,
				   const char* p_Component,
				   const char* p_Tag,
				   const char* p_Message,
				   const std::source_location& v_Location = std::source_location::current(),
				   bool v_ThreadTrace = true) noexcept {
			auto it = m_Loggers.find(p_Logger);
			if (it == m_Loggers.end()) return;

			Logger* pLogger = it->second;
			if (!pLogger) return;

			Tracing::LogEntry entry{ Tracing::LogLevel::CRASH, p_Component, p_Message, v_Location };
			pLogger->log(p_Tag, entry);

			if (v_ThreadTrace)
				Tracing::this_tracer::trace.pushFrame(entry);
		}

		// Only flush triggers actual emission inside loggers
		void flush() noexcept {
			for (auto& [code, logger] : m_Loggers) {
				if (logger) {
					logger->flush();
				}
			}
		}
	};
}
