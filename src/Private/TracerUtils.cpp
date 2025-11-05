#include "Stratum.h"
#include "TracerUtils.h"

namespace Stratum::Tracing {
	const char* LogEntry::getComponent() const {
		return this->m_Component;
	}

	std::source_location LogEntry::getLocation() const {
		return m_Location;
	}

	LogLevel LogEntry::getLogLevel() const {
		return m_Level;
	}

	std::string LogEntry::getMessage() const {
		return m_Message;
	}

	size_t LogEntry::getThreadID() const {
		return m_ThreadID;
	}

	std::chrono::steady_clock::time_point LogEntry::getTimestamp() const {
		return m_Timestamp;
	}

	void LogEntry::setThreadID(size_t v_ID) {
		m_ThreadID = v_ID;
	}

	void ExceptionProfile::setThreadID(size_t v_ID) {
		m_ThreadID = v_ID;
	}

	std::source_location ExceptionProfile::getLocation() const {
		return m_Location;
	}

	size_t ExceptionProfile::getThreadID() const {
		return m_ThreadID;
	}

	const char* ExceptionProfile::getComponent() const {
		return m_Component;
	}

	std::string ExceptionProfile::getErrorMessage() const {
		return m_ErrorMessage;
	}

	std::source_location TracingProfile::getLocation() const {
		return m_Location;
	}

	void TracingProfile::endTrace() {
		if (m_TraceStart) {
			m_End = std::chrono::steady_clock::now();
			m_TraceComplete = true;
		}
	}

	const char* TracingProfile::getComponent() const {
		return m_Component;
	}

	std::string TracingProfile::getLabel() const {
		return m_Label;
	}

	size_t TracingProfile::getThreadID() const {
		return m_ThreadID;
	}

	double TracingProfile::getTraceDuration(TracerPrecision v_Precision) const {
		if (m_TraceComplete)
			switch (v_Precision) {
			case TracerPrecision::SECONDS:
				return std::chrono::duration_cast<std::chrono::duration<double>>(m_End - m_Start).count();
			case TracerPrecision::MILLISECONDS:
				return std::chrono::duration_cast<std::chrono::duration<double, std::milli>>(m_End - m_Start).count();
			case TracerPrecision::MICROSECONDS:
				return std::chrono::duration_cast<std::chrono::duration<double, std::micro>>(m_End - m_Start).count();
			case TracerPrecision::NANOSECONDS:
				return std::chrono::duration_cast<std::chrono::duration<double, std::nano>>(m_End - m_Start).count();
			}
		return 0.0;
	}

	void TracingProfile::startTrace() {
		if (m_TraceStart) return;
		m_Start = std::chrono::steady_clock::now();
		m_TraceStart = true;
	}

	void TracingProfile::setThreadID(size_t v_ID) {
		m_ThreadID = v_ID;
	}


	void* LoggerProfile::getBuffer() const {
		return m_Buffer;
	}

	size_t LoggerProfile::getBufferSize() const {
		return m_BufferSize;
	}

	void LoggerProfile::enable() {
		m_Enabled = true;
	}

	void LoggerProfile::disable() {
		m_Enabled = false;
	}

	bool LoggerProfile::changeTracingPolicy(StackTracePolicy v_Policy) {
		if (v_Policy == m_TracePolicy) return false;
		m_TracePolicy = v_Policy;
		return true;
	}

	std::string LoggerProfile::getName() const {
		return m_Name;
	}

	bool LoggerProfile::isEnabled() const {
		return this->m_Enabled;
	}
}