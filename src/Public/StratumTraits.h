#pragma once
#include "Stratum.h"
#include "TracerUtils.h"

namespace Stratum::Traits {
	enum class RecordType : uint8_t {
		EXCEPTION, TRACE, LOG, NONE
	};

	class ProfileRecord {
		void* m_Memory;
		bool m_Init;
		RecordType m_Type;
		uint8_t m_AlignOffset;
	public:
		ProfileRecord();
		~ProfileRecord();

		ProfileRecord(const ProfileRecord& ro_Other);
		ProfileRecord& operator=(const ProfileRecord& ro_Other);
		ProfileRecord(ProfileRecord&& u_Other) noexcept;
		ProfileRecord& operator=(ProfileRecord&& u_Other) noexcept;

		template<typename T> requires
			std::is_same_v<T, Tracing::LogEntry> ||
			std::is_same_v<T, Tracing::TracingProfile> ||
			std::is_same_v<T, Tracing::ExceptionProfile>
			void emplace(const T& obj) {
			::new(m_Memory) T(obj);
			m_Init = true;
			if (std::is_same_v<T, Tracing::ExceptionProfile>) {
				m_Type = RecordType::EXCEPTION;
			} else if (std::is_same_v<T, Tracing::TracingProfile>) {
				m_Type = RecordType::TRACE;
			} else if (std::is_same_v<T, Tracing::LogEntry>) {
				m_Type = RecordType::LOG;
			}
		}

		RecordType kind() const;
		void destroy();

		template<typename T>
		T* get() {
			return m_Init ? (static_cast<T*>(m_Memory)) : nullptr;
		}
	};
}
