#include "../Public/Stratum.h"
#include "../Public/ProfilerTrace.h"

namespace Stratum::Profiler {

    

    ProfilerTrace::ProfilerTrace() noexcept = default;

    ProfilerTrace::ProfilerTrace(const ProfilerTrace& ro_Other) noexcept {
        copyFrom(ro_Other);
    }

    ProfilerTrace::ProfilerTrace(ProfilerTrace&& u_Other) noexcept {
        moveFrom(std::move(u_Other));
    }

    ProfilerTrace& ProfilerTrace::operator=(const ProfilerTrace& ro_Other) noexcept {
        if (this != &ro_Other) {
            destroy();
            copyFrom(ro_Other);
        }
        return *this;
    }

    ProfilerTrace& ProfilerTrace::operator=(ProfilerTrace&& u_Other) noexcept {
        if (this != &u_Other) {
            destroy();
            moveFrom(std::move(u_Other));
        }
        return *this;
    }

    ProfilerTrace::~ProfilerTrace() noexcept {
        destroy();
    }



    void ProfilerTrace::emplace(const Records::LogEntry& ro_Entry) noexcept {
        destroy();
        ::new(&m_storage.log) Records::LogEntry(ro_Entry);
        m_kind   = EntryKind::Log;
        m_active = true;
    }

    void ProfilerTrace::emplace(const Records::ExceptionEntry& ro_Entry) noexcept {
        destroy();
        ::new(&m_storage.exception) Records::ExceptionEntry(ro_Entry);
        m_kind   = EntryKind::Exception;
        m_active = true;
    }

    void ProfilerTrace::emplace(const Records::TracerEntry& ro_Entry) noexcept {
        destroy();
        ::new(&m_storage.tracer) Records::TracerEntry(ro_Entry);
        m_kind   = EntryKind::Tracer;
        m_active = true;
    }



    EntryKind ProfilerTrace::kind()   const noexcept { return m_kind; }
    bool      ProfilerTrace::active() const noexcept { return m_active; }

    void ProfilerTrace::destroy() noexcept {
        if (!m_active) return;
        switch (m_kind) {
            case EntryKind::Log:       m_storage.log.~LogEntry();               break;
            case EntryKind::Exception: m_storage.exception.~ExceptionEntry();   break;
            case EntryKind::Tracer:    m_storage.tracer.~TracerEntry();         break;
            case EntryKind::None:      break;
        }
        m_kind   = EntryKind::None;
        m_active = false;
    }



    void ProfilerTrace::copyFrom(const ProfilerTrace& ro_Other) noexcept {
        if (!ro_Other.m_active) return;
        switch (ro_Other.m_kind) {
            case EntryKind::Log:
                ::new(&m_storage.log) Records::LogEntry(ro_Other.m_storage.log);
                break;
            case EntryKind::Exception:
                ::new(&m_storage.exception) Records::ExceptionEntry(ro_Other.m_storage.exception);
                break;
            case EntryKind::Tracer:
                ::new(&m_storage.tracer) Records::TracerEntry(ro_Other.m_storage.tracer);
                break;
            case EntryKind::None:
                break;
        }
        m_kind   = ro_Other.m_kind;
        m_active = true;
    }

    void ProfilerTrace::moveFrom(ProfilerTrace&& u_Other) noexcept {
        if (!u_Other.m_active) return;
        switch (u_Other.m_kind) {
            case EntryKind::Log:
                ::new(&m_storage.log) Records::LogEntry(std::move(u_Other.m_storage.log));
                break;
            case EntryKind::Exception:
                ::new(&m_storage.exception) Records::ExceptionEntry(std::move(u_Other.m_storage.exception));
                break;
            case EntryKind::Tracer:
                ::new(&m_storage.tracer) Records::TracerEntry(std::move(u_Other.m_storage.tracer));
                break;
            case EntryKind::None:
                break;
        }
        m_kind   = u_Other.m_kind;
        m_active = true;

        u_Other.destroy();
    }

}
