#pragma once
#include "Stratum.h"
#include "Records.h"
#include "ProfilerTrace.h"
#include "Filter.h"
#include "Sinks.h"

namespace Stratum::Logging {

    
    
    
    

    class STRATUM_API ILogger {
    public:
        virtual void onLog(const char* p_Tag, const Records::LogEntry&       ro_Entry) = 0;
        virtual void onLog(const char* p_Tag, const Records::ExceptionEntry& ro_Entry) = 0;
        virtual void onLog(const char* p_Tag, const Records::TracerEntry&    ro_Entry) = 0;
        virtual void flush()                                                            = 0;
        virtual void addRoute(const char* p_Tag, ISinkRouter* po_Router)               = 0;

        virtual ~ILogger() = default;

        ILogger()                              = default;
        ILogger(const ILogger&)                = delete;
        ILogger& operator=(const ILogger&)     = delete;
        ILogger(ILogger&&)            noexcept = default;
        ILogger& operator=(ILogger&&) noexcept = default;
    };

    
    
    
    

    class STRATUM_API Logger final {
    private:
        ILogger* m_logger = nullptr;

    public:
        explicit Logger(ILogger* po_Logger) noexcept;

        STRATUM_NODISCARD bool valid() const noexcept;

        void log(const char* p_Tag, const Records::LogEntry&       ro_Entry) const noexcept;
        void log(const char* p_Tag, const Records::ExceptionEntry& ro_Entry) const noexcept;
        void log(const char* p_Tag, const Records::TracerEntry&    ro_Entry) const noexcept;
        void flush()                                                          const noexcept;
        void addRoute(const char* p_Tag, ISinkRouter* po_Router)             const noexcept;
    };

    

    enum class OverflowPolicy : uint8_t {
        DropOldest,  
        DropNewest   
    };

    
    
    
    
    
    
    
    
    

    template<size_t Capacity = 512>
    class DefaultLogger final : public ILogger {
        STRATUM_STATIC_ASSERT((Capacity & (Capacity - 1)) == 0,
            "DefaultLogger: Capacity must be a power of two");
        STRATUM_STATIC_ASSERT(Capacity > 0,
            "DefaultLogger: Capacity must be greater than zero");

    private:
        using RingEntry = std::pair<
            char[Records::TAG_MAX],
            Profiler::ProfilerTrace
        >;

        

        struct RouteEntry {
            char            m_tag[Records::TAG_MAX]{};
            ISinkRouter*    m_router = nullptr;
        };

        std::array<RingEntry,  Capacity>                m_ring{};
        std::array<RouteEntry, Records::MAX_ROUTES>     m_routes{};
        size_t          m_head          = 0;
        size_t          m_tail          = 0;
        size_t          m_count         = 0;
        size_t          m_routeCount    = 0;
        OverflowPolicy  m_overflow      = OverflowPolicy::DropOldest;
        Filtering::FilterChain  m_filter{};
        Records::LoggerProfile* m_profile = nullptr;

        static constexpr size_t MASK = Capacity - 1;

        STRATUM_FORCEINLINE bool isFull()  const noexcept { return m_count == Capacity; }
        STRATUM_FORCEINLINE bool isEmpty() const noexcept { return m_count == 0; }

        void storeEntry(const char* p_Tag, const Profiler::ProfilerTrace& ro_Trace) noexcept {
            if (isFull()) {
                if (m_overflow == OverflowPolicy::DropNewest) return;
                
                m_head = (m_head + 1) & MASK;
                --m_count;
            }

            auto& slot = m_ring[m_tail];
            std::strncpy(slot.first, p_Tag, Records::TAG_MAX - 1);
            slot.first[Records::TAG_MAX - 1] = '\0';
            slot.second = ro_Trace;

            m_tail = (m_tail + 1) & MASK;
            ++m_count;
        }

    public:
        explicit DefaultLogger(
            OverflowPolicy              v_Overflow = OverflowPolicy::DropOldest,
            Records::LoggerProfile*     po_Profile = nullptr
        ) noexcept
            : m_overflow(v_Overflow), m_profile(po_Profile) {}

        

        STRATUM_NODISCARD Filtering::FilterChain& filter() noexcept { return m_filter; }

        

        void onLog(const char* p_Tag, const Records::LogEntry& ro_Entry) noexcept override {
            if (STRATUM_UNLIKELY(!m_filter.accepts(ro_Entry.getLevel(),
                                                    ro_Entry.getComponent(), p_Tag)))
                return;

            Profiler::ProfilerTrace trace;
            trace.emplace(ro_Entry);
            storeEntry(p_Tag, trace);
        }

        void onLog(const char* p_Tag, const Records::ExceptionEntry& ro_Entry) noexcept override {
            if (STRATUM_UNLIKELY(!m_filter.accepts(Records::LogLevel::Error,
                                                    ro_Entry.getComponent(), p_Tag)))
                return;

            Profiler::ProfilerTrace trace;
            trace.emplace(ro_Entry);
            storeEntry(p_Tag, trace);
        }

        void onLog(const char* p_Tag, const Records::TracerEntry& ro_Entry) noexcept override {
            if (STRATUM_UNLIKELY(!m_filter.accepts(Records::LogLevel::Info,
                                                    ro_Entry.getComponent(), p_Tag)))
                return;

            Profiler::ProfilerTrace trace;
            trace.emplace(ro_Entry);
            storeEntry(p_Tag, trace);
        }

        void addRoute(const char* p_Tag, ISinkRouter* po_Router) noexcept override {
            if (STRATUM_UNLIKELY(!p_Tag || !po_Router)) return;
            if (STRATUM_UNLIKELY(m_routeCount >= Records::MAX_ROUTES)) return;

            auto& entry = m_routes[m_routeCount++];
            std::strncpy(entry.m_tag, p_Tag, Records::TAG_MAX - 1);
            entry.m_tag[Records::TAG_MAX - 1] = '\0';
            entry.m_router = po_Router;
        }

        STRATUM_COLD void flush() noexcept override {
            while (!isEmpty()) {
                auto& slot = m_ring[m_head];
                const char*                 tag   = slot.first;
                Profiler::ProfilerTrace&    trace = slot.second;

                ISinkRouter* router = nullptr;
                for (size_t i = 0; i < m_routeCount; ++i) {
                    if (std::strncmp(m_routes[i].m_tag, tag, Records::TAG_MAX) == 0) {
                        router = m_routes[i].m_router;
                        break;
                    }
                }

                if (STRATUM_LIKELY(router != nullptr)) {
                    switch (trace.kind()) {
                        case Profiler::EntryKind::Log:
                            router->write(*trace.get<Records::LogEntry>());
                            break;
                        case Profiler::EntryKind::Exception:
                            router->write(*trace.get<Records::ExceptionEntry>());
                            break;
                        case Profiler::EntryKind::Tracer:
                            router->write(*trace.get<Records::TracerEntry>());
                            break;
                        case Profiler::EntryKind::None:
                            break;
                    }
                }

                trace.destroy();
                m_head = (m_head + 1) & MASK;
                --m_count;
            }
        }
    };

}
