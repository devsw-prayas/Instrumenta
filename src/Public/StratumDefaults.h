#pragma once
#include "LogRoutes.h"
#include "StratumTraits.h"
#include "TracerUtils.h"

namespace Stratum::Logging {
	class STRATUM DefaultLogger final : public ILogger<DefaultLogger> {
		Tracing::LoggerProfile& m_Profile;
		std::vector<std::pair<std::string_view, Traits::ProfileRecord>> m_Records;
		std::unordered_map<std::string_view, ISinkRouter*> m_Routes; // tag → route

	public:
		explicit DefaultLogger(Tracing::LoggerProfile& r_Profile) 
			: m_Profile(r_Profile) {
			m_Records.reserve(256); // small static buffer for startup logs
		}

		// Add a route for a specific tag
		void registerRoute(const char* p_Tag, ISinkRouter* p_Route) noexcept {
			if (p_Tag && p_Route)
				m_Routes.emplace(p_Tag, p_Route);
		}

		// Collects a onLogC entry into the record list
		void onLogC(const char* p_Tag, const Tracing::LogEntry& ro_Entry) noexcept {
			if (!m_Profile.isEnabled()) return;

			Traits::ProfileRecord record;
			record.emplace(ro_Entry);
			m_Records.emplace_back(p_Tag, std::move(record));
		}

		// Collects a trace entry
		void onLogC(const char* p_Tag, const Tracing::TracingProfile& ro_Trace) noexcept {
			if (!m_Profile.isEnabled()) return;

			Traits::ProfileRecord record;
			record.emplace(ro_Trace);
			m_Records.emplace_back(p_Tag, std::move(record));
		}

		// Collects an exception entry
		void onLogC(const char* p_Tag, const Tracing::ExceptionProfile& ro_Exception) noexcept {
			if (!m_Profile.isEnabled()) return;

			Traits::ProfileRecord record;
			record.emplace(ro_Exception);
			m_Records.emplace_back(p_Tag, std::move(record));
		}

		// Flush: route all stored records, then clear
		void flushC() noexcept {
			for (auto& record : m_Records) {
				std::string_view tag = record.first;
				auto it = m_Routes.find(tag);
				if (it != m_Routes.end() && it->second)
					switch (record.second.kind()) {
					case Traits::RecordType::EXCEPTION:
						it->second->write(*record.second.get<Tracing::ExceptionProfile>());
						break;
					case  Traits::RecordType::LOG:
						it->second->write(*record.second.get<Tracing::LogEntry>());
						break;
					case Traits::RecordType::TRACE:
						it->second->write(*record.second.get<Tracing::TracingProfile>());
						break;
					case  Traits::RecordType::NONE:; // No op
						break;
					}
			}
			m_Records.clear();
		}
	};
}