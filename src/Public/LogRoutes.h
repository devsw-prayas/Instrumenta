#pragma once
#include "TracerUtils.h"

namespace Stratum::Logging {
	class ISink {
	public:
		virtual void sink(std::string_view v_Payload) = 0;
		virtual ~ISink() = default;

		ISink(const ISink&) = delete;
		ISink& operator=(const ISink&) = delete;

		ISink(ISink&&) noexcept = default;
		ISink& operator=(ISink&&) noexcept = default;

	};

	class ISinkRouter {
	public:
		virtual void write(const Tracing::LogEntry& ro_Entry) = 0;
		virtual void write(const Tracing::ExceptionProfile& ro_ExceptionProfile) = 0;
		virtual void write(const Tracing::TracingProfile& ro_TracingProfile) = 0;
		virtual ~ISinkRouter() = default;

		ISinkRouter(const ISinkRouter&) = delete;
		ISinkRouter& operator=(const ISinkRouter&) = delete;

		ISinkRouter(ISinkRouter&&) noexcept = default;
		ISinkRouter& operator=(ISinkRouter&&) noexcept = default;
	};

	template<typename T>
	class ILogger {
	public:
		void onLog(const char* p_Tag, const Tracing::LogEntry& ro_Entry) {
			static_cast<T*>(this)->onLogC(p_Tag, ro_Entry);
		}

		void onLog(const char* p_Tag, const Tracing::ExceptionProfile& ro_ExceptionProfile) {
			static_cast<T*>(this)->onLogC(p_Tag, ro_ExceptionProfile);
		}

		void onLog(const char* p_Tag, const Tracing::TracingProfile& ro_TracingProfile) {
			static_cast<T*>(this)->onLogC(p_Tag, ro_TracingProfile);
		}

		void flush() {
			static_cast<T*>(this)->flushC();
		}

		void addRoute(ISinkRouter* po_Route) {
			static_cast<T*>(this)->addRouteC(po_Route);
		}

		ILogger() = default;

		virtual ~ILogger() = default;

		ILogger(const ILogger&) = delete;
		ILogger& operator=(const ILogger&) = delete;

		ILogger(ILogger&&) noexcept = default;
		ILogger& operator=(ILogger&&) noexcept = default;
	};

	class Logger {
		void* m_Self;
		void (*m_FunctionLogEntry)(void*, const char* p_Tag, const Tracing::LogEntry&);
		void (*m_FunctionExceptionProfile)(void*, const char* p_Tag, const Tracing::ExceptionProfile&);
		void (*m_FunctionTracingProfile)(void*, const char* p_Tag, const Tracing::TracingProfile&);
		void (*m_FunctionFlush)(void*);

	public:
		template<typename L>
		explicit Logger(L& logger)
			: m_Self(&logger),
			m_FunctionLogEntry([](void* p, const char* p_Tag, const Tracing::LogEntry& e) {
			static_cast<L*>(p)->onLog(p_Tag, e);
		}),
			m_FunctionExceptionProfile([](void* p, const char* p_Tag, const Tracing::ExceptionProfile& e) {
			static_cast<L*>(p)->onLog(p_Tag, e);
		}),
			m_FunctionTracingProfile([](void* p, const char* p_Tag, const Tracing::TracingProfile& e) {
			static_cast<L*>(p)->onLog(p_Tag, e);
		}),
			m_FunctionFlush([](void* p) {
			static_cast<L*>(p)->flush();
		}) {
		}

		void log(const char* p_Tag, const Tracing::LogEntry& e) const noexcept { m_FunctionLogEntry(m_Self, p_Tag, e); }
		void log(const char* p_Tag, const Tracing::ExceptionProfile& e) const noexcept { m_FunctionExceptionProfile(m_Self, p_Tag, e); }
		void log(const char* p_Tag, const Tracing::TracingProfile& e) const noexcept { m_FunctionTracingProfile(m_Self, p_Tag, e); }
		void flush() const noexcept { m_FunctionFlush(m_Self); }
	};
}
