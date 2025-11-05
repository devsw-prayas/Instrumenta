#include "Stratum.h"
#include "StratumTraits.h"

namespace Stratum::Traits {

    static void* allocateAligned128(uint8_t& r_Offset) noexcept {
        // Step 1: allocate base block
        void* base = std::malloc(128);
        if (!base) {
            r_Offset = 0;
            return nullptr;
        }

        // Step 2: compute offset for 128B alignment
        uintptr_t addr = reinterpret_cast<uintptr_t>(base);
        uintptr_t offset = ((addr + 127ULL) & ~127ULL) - addr;
        r_Offset = static_cast<uint8_t>(offset);

        // Step 3: resize block to hold padding region safely
        void* resized = std::realloc(base, 128 + offset);
        if (!resized) {
            std::free(base);
            r_Offset = 0;
            return nullptr;
        }

        // Step 4: shift pointer to aligned address within resized block
        std::byte* alignedPtr = static_cast<std::byte*>(resized) + offset;
        return alignedPtr;
    }

    ProfileRecord::ProfileRecord()
        : m_Memory(nullptr), m_Init(false), m_Type(RecordType::NONE), m_AlignOffset(0) {
        m_Memory = allocateAligned128(m_AlignOffset);
    }

    ProfileRecord::ProfileRecord(const ProfileRecord& ro_Other)
        : m_Memory(nullptr), m_Init(false), m_Type(RecordType::NONE), m_AlignOffset(0) {
        m_Memory = allocateAligned128(m_AlignOffset);
        if (!m_Memory || !ro_Other.m_Init) return;

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

    ProfileRecord::ProfileRecord(ProfileRecord&& u_Other) noexcept
        : m_Memory(nullptr), m_Init(false), m_Type(RecordType::NONE), m_AlignOffset(0) {
        m_Memory = allocateAligned128(m_AlignOffset);
        if (!m_Memory || !u_Other.m_Init) return;

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

        // reset moved-from
        u_Other.m_Memory = nullptr;
        u_Other.m_Init = false;
        u_Other.m_Type = RecordType::NONE;
        u_Other.m_AlignOffset = 0;
    }

    ProfileRecord& ProfileRecord::operator=(const ProfileRecord& ro_Other) {
        if (this != &ro_Other) {
            destroy();
            if (!m_Memory)
                m_Memory = allocateAligned128(m_AlignOffset);

            if (!m_Memory || !ro_Other.m_Init) return *this;

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

    ProfileRecord& ProfileRecord::operator=(ProfileRecord&& u_Other) noexcept {
        if (this != &u_Other) {
            destroy();
            if (!m_Memory)
                m_Memory = allocateAligned128(m_AlignOffset);

            if (!m_Memory || !u_Other.m_Init) return *this;

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

            u_Other.m_Memory = nullptr;
            u_Other.m_Init = false;
            u_Other.m_Type = RecordType::NONE;
            u_Other.m_AlignOffset = 0;
        }
        return *this;
    }

    ProfileRecord::~ProfileRecord() {
        destroy();
        if (m_Memory) {
            void* basePtr = static_cast<std::byte*>(m_Memory) - m_AlignOffset;
            std::free(basePtr);
            m_Memory = nullptr;
        }
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
