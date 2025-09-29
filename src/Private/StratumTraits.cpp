#include "Stratum.h"
#include "StratumTraits.h"

namespace Stratum::Traits {
	ProfileRecord::ProfileRecord(ProfileRecord&& u_Other) noexcept {
		switch (u_Other.m_Type) {
			case RecordType::LOG:
				::new(m_Memory) Tracing::LogEntry(std::move(*static_cast<Tracing::LogEntry*>(u_Other.m_Memory)));
				break;
			case RecordType::EXCEPTION:
				::new(m_Memory) Tracing::ExceptionProfile(std::move(*static_cast<Tracing::ExceptionProfile*>(u_Other.m_Memory)));
				break;
			case RecordType::TRACE:
				::new(m_Memory) Tracing::TracingProfile(std::move(*static_cast<Tracing::TracingProfile*>(u_Other.m_Memory)));
				break;
			case RecordType::NONE:
				break;
		}
		m_Type = u_Other.m_Type;
		m_Init = u_Other.m_Init;

	}

	ProfileRecord::ProfileRecord(const ProfileRecord& ro_Other) {
		switch (ro_Other.m_Type) {
			case RecordType::LOG:
				::new(m_Memory) Tracing::LogEntry(*static_cast<Tracing::LogEntry*>(ro_Other.m_Memory));
				break;
			case RecordType::EXCEPTION:
				::new(m_Memory) Tracing::ExceptionProfile(*static_cast<Tracing::ExceptionProfile*>(ro_Other.m_Memory));
				break;
			case RecordType::TRACE:
				::new(m_Memory) Tracing::TracingProfile(*static_cast<Tracing::TracingProfile*>(ro_Other.m_Memory));
				break;
			case RecordType::NONE:
				break;
		}
		m_Type = ro_Other.m_Type;
		m_Init = ro_Other.m_Init;
	}

	ProfileRecord& ProfileRecord::operator=(ProfileRecord&& u_Other) noexcept {
		if (this != &u_Other) {
			destroy();
			switch (u_Other.m_Type) {
				case RecordType::LOG:
					::new(m_Memory) Tracing::LogEntry(std::move(*static_cast<Tracing::LogEntry*>(u_Other.m_Memory)));
					break;
				case RecordType::EXCEPTION:
					::new(m_Memory) Tracing::ExceptionProfile(std::move(*static_cast<Tracing::ExceptionProfile*>(u_Other.m_Memory)));
					break;
				case RecordType::TRACE:
					::new(m_Memory) Tracing::TracingProfile(std::move(*static_cast<Tracing::TracingProfile*>(u_Other.m_Memory)));
					break;
				case RecordType::NONE:
					break;
			}
			m_Type = u_Other.m_Type;
			m_Init = u_Other.m_Init;

		}
		return *this;
	}

	ProfileRecord& ProfileRecord::operator=(const ProfileRecord& ro_Other) {
		if (this != &ro_Other)
		{
			destroy();
			switch (ro_Other.m_Type) {
				case RecordType::LOG:
					::new(m_Memory) Tracing::LogEntry(*static_cast<Tracing::LogEntry*>(ro_Other.m_Memory));
					break;
				case RecordType::EXCEPTION:
					::new(m_Memory) Tracing::ExceptionProfile(*static_cast<Tracing::ExceptionProfile*>(ro_Other.m_Memory));
					break;
				case RecordType::TRACE:
					::new(m_Memory) Tracing::TracingProfile(*static_cast<Tracing::TracingProfile*>(ro_Other.m_Memory));
					break;
				case RecordType::NONE:
					break;
			}
			m_Type = ro_Other.m_Type;
			m_Init = ro_Other.m_Init;

		}
		return *this;
	}

	ProfileRecord::ProfileRecord() {
		void* t = malloc(128);
		uintptr_t mem = reinterpret_cast<uintptr_t>(t);
		mem = ((mem + (127ULL)) & ~127ULL) - mem;
		m_AlignOffset = static_cast<uint8_t>(mem);
		m_Memory = static_cast<std::byte*>(realloc(t, 128 + mem)) + mem;
		m_Init = false;
		m_Type = RecordType::NONE;
	}

	ProfileRecord::~ProfileRecord() {
		destroy();
		void* basePtr = static_cast<std::byte*>(m_Memory) - m_AlignOffset;
		free(basePtr);
	}


	void ProfileRecord::destroy() {
		if (!m_Init) return;

		switch (m_Type) {
			case RecordType::LOG:
				static_cast<Tracing::LogEntry*>(m_Memory)->~LogEntry();
				break;
			case RecordType::EXCEPTION:
				static_cast<Tracing::ExceptionProfile*>(m_Memory)->~ExceptionProfile();
				break;
			case RecordType::TRACE:
				static_cast<Tracing::TracingProfile*>(m_Memory)->~TracingProfile();
				break;
			case RecordType::NONE:
				break;
		}

		m_Type = RecordType::NONE;
		m_Init = false;
	}


	RecordType ProfileRecord::kind() const {
		return m_Type;
	}
}