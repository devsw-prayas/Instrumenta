#pragma once
#include "Stratum.h"
#include "LogRoutes.h"
#include "StackTrace.h"

namespace Stratum::Logging {

	class STRATUM Orchestrator final {
		std::vector<Logger> loggers;
	public:
		static Orchestrator& getInstance() {
			static Orchestrator s_Instance;
			return s_Instance;
		}

		void registerLogger(Logger& ro_Logger) {
			
		}

		template<typename T = void>
		void log(Tracing::LogLevel v_Level, const char* p_Component, const char* p_Tag, 
			const char* p_Message, const std::source_location v_Location = std::source_location::current(), T* trace) {
			
		}

};	
}
