#pragma once
#include "Stratum.h"

namespace Stratum::Tracing {
	enum class LogLevel : uint8_t {
		INFO, WARNING, DEBUG, ERROR, CRASH
	};

	enum class StackTracePolicy : uint8_t {
		APPEND_ONLY, AUTO_FLUSH_ON_ERROR, AUTO_FLUSH_ON_CRASH, AUTO_FLUSH_ALWAYS
	};

	// [Size in Bytes]: 72
	struct STRATUM alignas(128) LogEntry final {
	private:
		LogLevel m_Level;
		const char* m_Component;
		const char* m_Tag;
		const char* m_Message;
		std::chrono::steady_clock::time_point m_Timestamp;
		std::source_location m_Location;
		uint64_t m_ThreadID;

	public:
		LogEntry(LogLevel v_Level, const char* p_Tag, const char* p_Component, const char* p_Message, const std::source_location& ro_location = std::source_location::current()) :
			m_Level(v_Level), m_Component(p_Component), m_Tag(p_Tag), m_Message(p_Message), m_Timestamp(std::chrono::steady_clock::now()), m_Location(ro_location), m_ThreadID(0) {
		}

		const char* getComponent();
		const char* getTag();
		const char* getMessage();
		LogLevel getLogLevel();
		std::chrono::steady_clock::time_point getTimestamp();
		std::source_location getLocation();
		void setThreadID(size_t v_ID);
		size_t getThreadID();
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
		const char* getComponent();
		const char* getErrorMessage();
		std::source_location getLocation();
		void setThreadID(size_t v_ID);
		size_t getThreadID();
	};

	// [Size in Bytes]: 80
	struct STRATUM alignas(128) TracingProfile final {
	private:
		const char* m_Component;
		const char* m_Label;
		double m_DurationMS;
		std::chrono::steady_clock::time_point m_Start;
		std::chrono::steady_clock::time_point m_End;
		std::source_location m_Location;
		uint64_t m_ThreadID;
		bool m_TraceComplete;
	public:
		TracingProfile(const char* p_Component, const char* p_Label, const std::source_location& ro_Location = std::source_location::current()) :
			m_Component(p_Component), m_Label(p_Label), m_DurationMS(0.f), m_Location(ro_Location), m_ThreadID(0), m_TraceComplete(false) {
		}

		const char* getComponent();
		std::source_location getLocation();
		void setThreadID();
		size_t getThreadID(size_t v_ID);
		const char* getLabel();
		double getTraceDuration();
		void startTrace();
		void endTrace();
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
		void* getBuffer();
		size_t getBufferSize();
		void enable();
		void disable();
		bool changeTracingPolicy(StackTracePolicy v_Policy);
	};
}
