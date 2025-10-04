#pragma once
#include "TracerUtils.h"

namespace Stratum::Logging {
	class ISink {
	public:
		virtual void sink(std::string_view v_Payload) = 0;
		virtual ~ISink();
	};

	class ISinkRouter {
	public:
		virtual void write(const Tracing::LogEntry& ro_Entry) = 0;
		virtual void write(const Tracing::ExceptionProfile& ro_ExceptionProfile) = 0;
		virtual void write(const Tracing::TracingProfile& ro_TracingProfile) = 0;
		virtual ~ISinkRouter();
	};

	template<typename T>
	class ILogger {
		void onLog(const Tracing::LogEntry& ro_Entry) {
			static_cast<T*>(this)->onLogC(ro_Entry);
		}

		void onLog(const Tracing::ExceptionProfile& ro_ExceptionProfile) {
			static_cast<T*>(this)->onLogC(ro_ExceptionProfile);
		}

		void onLog(const Tracing::TracingProfile& ro_TracingProfile) {
			static_cast<T*>(this)->onLogC(ro_TracingProfile);
		}

		void addRoute(ISinkRouter* po_Route) {
			static_cast<T*>(this)->addRouteC(po_Route);
		}
	};

	class Logger{
		void* m_Self;
		void (*m_FunctionLogEntry)(void*, const Tracing::LogEntry&);
		void (*m_FunctionExceptionProfile)(void*, const Tracing::ExceptionProfile&);
		void (*m_FunctionTracingProfile)(void*, const Tracing::TracingProfile&);

	public:
		template<typename L>
		explicit Logger(L& logger)
			: m_Self(&logger),
			m_FunctionLogEntry([](void* p, const Tracing::LogEntry& e) {
			static_cast<L*>(p)->onLog(e);
			}),
			m_FunctionExceptionProfile([](void* p, const Tracing::ExceptionProfile& e) {
			static_cast<L*>(p)->onLog(e);
			}),
			m_FunctionTracingProfile([](void* p, const Tracing::TracingProfile& e) {
			static_cast<L*>(p)->onLog(e);
			}) {}

		void log(const Tracing::LogEntry& e) const noexcept { m_FunctionLogEntry(m_Self, e); }
		void log(const Tracing::ExceptionProfile& e) const noexcept { m_FunctionExceptionProfile(m_Self, e); }
		void log(const Tracing::TracingProfile& e) const noexcept { m_FunctionTracingProfile(m_Self, e); }
	};

}
