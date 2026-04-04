#pragma once
#include "Stratum.h"
#include "Records.h"

namespace Stratum::Profiler {
    enum class EntryKind : uint8_t {
        None, Log, Exception, Tracer
    };

    class STRATUM_API ProfilerTrace final {
    private:
        union Storage {
            Records::LogEntry       log;
            Records::ExceptionEntry exception;
            Records::TracerEntry    tracer;
            Storage()  noexcept {}
            ~Storage() noexcept {}
        };

        Storage   m_storage;
        EntryKind m_kind   = EntryKind::None;
        bool      m_active = false;

        void copyFrom(const ProfilerTrace& ro_Other) noexcept;
        void moveFrom(ProfilerTrace&& u_Other)       noexcept;

    public:
        ProfilerTrace()                                          noexcept;
        ProfilerTrace(const ProfilerTrace& ro_Other)             noexcept;
        ProfilerTrace(ProfilerTrace&& u_Other)                   noexcept;
        ProfilerTrace& operator=(const ProfilerTrace& ro_Other)  noexcept;
        ProfilerTrace& operator=(ProfilerTrace&& u_Other)        noexcept;
        ~ProfilerTrace()                                         noexcept;

        void emplace(const Records::LogEntry& ro_Entry)       noexcept;
        void emplace(const Records::ExceptionEntry& ro_Entry) noexcept;
        void emplace(const Records::TracerEntry& ro_Entry)    noexcept;

        template<typename T>
        STRATUM_NODISCARD_MSG("Ignoring get() discards the retrieved entry pointer")
        T* get() noexcept {
            if (STRATUM_UNLIKELY(!m_active)) return nullptr;

            if constexpr (std::is_same_v<T, Records::LogEntry>)
                return m_kind == EntryKind::Log       ? &m_storage.log       : nullptr;
            else if constexpr (std::is_same_v<T, Records::ExceptionEntry>)
                return m_kind == EntryKind::Exception ? &m_storage.exception : nullptr;
            else if constexpr (std::is_same_v<T, Records::TracerEntry>)
                return m_kind == EntryKind::Tracer    ? &m_storage.tracer    : nullptr;
            else {
                static_assert(!sizeof(T), "ProfilerTrace::get<T>() -- unsupported type T");
                return nullptr;
            }
        }

        template<typename T>
        STRATUM_NODISCARD_MSG("Ignoring get() discards the retrieved entry pointer")
        const T* get() const noexcept {
            return const_cast<ProfilerTrace*>(this)->get<T>();
        }

        STRATUM_NODISCARD EntryKind kind()   const noexcept;
        STRATUM_NODISCARD bool      active() const noexcept;

        void destroy() noexcept;
    };
}
