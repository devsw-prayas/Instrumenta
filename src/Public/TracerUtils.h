#pragma once
#include "Stratum.h"

namespace Stratum::Tracing {
	enum class LogLevel : uint8_t {
		INFO, WARNING, DEBUG, ERROR, CRASH
	};

	enum class StackTracePolicy : uint8_t {
		APPEND_ONLY, AUTO_FLUSH_ON_ERROR, AUTO_FLUSH_ON_CRASH, AUTO_FLUSH_ALWAYS
	};

	enum class TracerPrecision : uint8_t {
		SECONDS, MILLISECONDS, MICROSECONDS, NANOSECONDS
	};

	// [Size in Bytes]: 64
	struct STRATUM alignas(128) LogEntry final {
	private:
		LogLevel m_Level;
		const char* m_Component;
		const char* m_Message;
		std::chrono::steady_clock::time_point m_Timestamp;
		std::source_location m_Location;
		uint64_t m_ThreadID;

	public:
		LogEntry(LogLevel v_Level, const char* p_Component, const char* p_Message, const std::source_location& ro_location = std::source_location::current()) :
			m_Level(v_Level), m_Component(p_Component), m_Message(p_Message), m_Timestamp(std::chrono::steady_clock::now()), m_Location(ro_location), m_ThreadID(0) {
		}

		const char* getComponent() const;
		const char* getMessage() const;
		LogLevel getLogLevel() const;
		std::chrono::steady_clock::time_point getTimestamp() const;
		std::source_location getLocation() const;
		void setThreadID(size_t v_ID);
		size_t getThreadID() const;

		static LogEntry s_Invalid;
	};

	// [Size in Bytes]: 56
	struct STRATUM alignas(128) ExceptionProfile final {
	private:
		const char* m_Component;
		const char* m_ErrorMessage;
		std::source_location m_Location;
		std::chrono::steady_clock::time_point m_Timestamp;
		uint64_t m_ThreadID;

	public:
		ExceptionProfile(const char* p_Component, const char* p_ErrorMessage, const std::source_location& ro_location = std::source_location::current()) :
			m_Component(p_Component), m_ErrorMessage(p_ErrorMessage), m_Location(ro_location), m_Timestamp(std::chrono::steady_clock::now()), m_ThreadID(0) {
		}
		const char* getComponent() const;
		const char* getErrorMessage() const;
		std::source_location getLocation() const;
		void setThreadID(size_t v_ID);
		size_t getThreadID() const;

		static ExceptionProfile s_Invalid;
	};

	// [Size in Bytes]: 72
	struct STRATUM alignas(128) TracingProfile final {
	private:
		const char* m_Component;
		const char* m_Label;
		std::chrono::steady_clock::time_point m_Start;
		std::chrono::steady_clock::time_point m_End;
		std::source_location m_Location;
		uint64_t m_ThreadID;
		bool m_TraceComplete;
		bool m_TraceStart;
	public:
		TracingProfile(const char* p_Component, const char* p_Label, const std::source_location& ro_Location = std::source_location::current()) :
			m_Component(p_Component), m_Label(p_Label), m_Location(ro_Location), m_ThreadID(0), m_TraceComplete(false), m_TraceStart(false) {
		}

		const char* getComponent() const;
		std::source_location getLocation() const;
		void setThreadID(size_t v_ID);
		size_t getThreadID() const;
		const char* getLabel() const;
		double getTraceDuration(TracerPrecision v_Precision) const;
		void startTrace();
		void endTrace();

		static TracingProfile s_Invalid;
	};
	 
	// [Size in Bytes]: 32
	struct LoggerProfile final {
	private:
		const char* m_Name;
		void* m_Buffer;
		size_t m_BufferSize;
		bool m_Enabled;
		StackTracePolicy m_TracePolicy;

	public:
		LoggerProfile(const char* p_Name, void* p_Buffer, size_t v_BufferSize) :
			m_Name(p_Name), m_Buffer(p_Buffer), m_BufferSize(v_BufferSize), m_Enabled(true), m_TracePolicy(StackTracePolicy::AUTO_FLUSH_ALWAYS) {
		}
		const char* getName() const;
		void* getBuffer() const;
		size_t getBufferSize() const;
		void enable();
		void disable();
		bool changeTracingPolicy(StackTracePolicy v_Policy);
	};
}
