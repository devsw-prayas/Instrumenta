#include "../Public/Stratum.h"
#include "../Public/ProfilerTrace.h"

namespace Stratum::Profiler {

    

    static void destroyStorage(
        void*       p_Storage,
        EntryKind   v_Kind
    ) noexcept {
        switch (v_Kind) {
            case EntryKind::Log:
                std::launder(reinterpret_cast<Records::LogEntry*>(p_Storage))
                    ->~LogEntry();
                break;
            case EntryKind::Exception:
                std::launder(reinterpret_cast<Records::ExceptionEntry*>(p_Storage))
                    ->~ExceptionEntry();
                break;
            case EntryKind::Tracer:
                std::launder(reinterpret_cast<Records::TracerEntry*>(p_Storage))
                    ->~TracerEntry();
                break;
            case EntryKind::None:
                break;
        }
    }

    static void copyStorage(
        void*       p_Dst,
        const void* p_Src,
        EntryKind   v_Kind
    ) noexcept {
        switch (v_Kind) {
            case EntryKind::Log:
                ::new(p_Dst) Records::LogEntry(
                    *std::launder(reinterpret_cast<const Records::LogEntry*>(p_Src)));
                break;
            case EntryKind::Exception:
                ::new(p_Dst) Records::ExceptionEntry(
                    *std::launder(reinterpret_cast<const Records::ExceptionEntry*>(p_Src)));
                break;
            case EntryKind::Tracer:
                ::new(p_Dst) Records::TracerEntry(
                    *std::launder(reinterpret_cast<const Records::TracerEntry*>(p_Src)));
                break;
            case EntryKind::None:
                break;
        }
    }

    static void moveStorage(
        void*       p_Dst,
        void*       p_Src,
        EntryKind   v_Kind
    ) noexcept {
        switch (v_Kind) {
            case EntryKind::Log:
                ::new(p_Dst) Records::LogEntry(
                    std::move(*std::launder(reinterpret_cast<Records::LogEntry*>(p_Src))));
                break;
            case EntryKind::Exception:
                ::new(p_Dst) Records::ExceptionEntry(
                    std::move(*std::launder(reinterpret_cast<Records::ExceptionEntry*>(p_Src))));
                break;
            case EntryKind::Tracer:
                ::new(p_Dst) Records::TracerEntry(
                    std::move(*std::launder(reinterpret_cast<Records::TracerEntry*>(p_Src))));
                break;
            case EntryKind::None:
                break;
        }
    }

    

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
        ::new(&m_storage) Records::LogEntry(ro_Entry);
        m_kind   = EntryKind::Log;
        m_active = true;
    }

    void ProfilerTrace::emplace(const Records::ExceptionEntry& ro_Entry) noexcept {
        destroy();
        ::new(&m_storage) Records::ExceptionEntry(ro_Entry);
        m_kind   = EntryKind::Exception;
        m_active = true;
    }

    void ProfilerTrace::emplace(const Records::TracerEntry& ro_Entry) noexcept {
        destroy();
        ::new(&m_storage) Records::TracerEntry(ro_Entry);
        m_kind   = EntryKind::Tracer;
        m_active = true;
    }

    

    EntryKind ProfilerTrace::kind()   const noexcept { return m_kind; }
    bool      ProfilerTrace::active() const noexcept { return m_active; }

    void ProfilerTrace::destroy() noexcept {
        if (!m_active) return;
        destroyStorage(&m_storage, m_kind);
        m_kind   = EntryKind::None;
        m_active = false;
    }

    

    void ProfilerTrace::copyFrom(const ProfilerTrace& ro_Other) noexcept {
        if (!ro_Other.m_active) return;
        copyStorage(&m_storage, &ro_Other.m_storage, ro_Other.m_kind);
        m_kind   = ro_Other.m_kind;
        m_active = true;
    }

    void ProfilerTrace::moveFrom(ProfilerTrace&& u_Other) noexcept {
        if (!u_Other.m_active) return;
        moveStorage(&m_storage, &u_Other.m_storage, u_Other.m_kind);
        m_kind   = u_Other.m_kind;
        m_active = true;

        destroyStorage(&u_Other.m_storage, u_Other.m_kind);
        u_Other.m_kind   = EntryKind::None;
        u_Other.m_active = false;
    }

}
