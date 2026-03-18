#pragma once
#include "Stratum.h"
#include "Records.h"
#include "ProfilerTrace.h"

namespace Stratum::Tracing {

    

    using StackFrame = Profiler::ProfilerTrace;

    

    struct Frame {
        Frame*      m_next  = nullptr;
        StackFrame  m_frame{};
    };

    

    struct STRATUM_API DefaultAllocator final {
        template<typename T>
        STRATUM_NODISCARD T* allocate(size_t v_Count) const noexcept {
            return static_cast<T*>(std::malloc(sizeof(T) * v_Count));
        }

        void deallocate(void* p_Memory) const noexcept {
            std::free(p_Memory);
        }

        template<typename T>
        void destroy(T* p_Object) const noexcept {
            p_Object->~T();
        }
    };

    
    
    
    

    template<typename A = DefaultAllocator>
    class StackTrace final {
    private:
        using Allocator = A;

        Frame*    m_sentinel = nullptr;
        Frame*    m_tail     = nullptr;
        Allocator m_allocator{};

    public:
        StackTrace() {
            m_sentinel = m_allocator.template allocate<Frame>(1);
            STRATUM_ASSERT(m_sentinel != nullptr);
            ::new(m_sentinel) Frame{};
            m_tail = m_sentinel;
        }

        StackTrace(const StackTrace&)            = delete;
        StackTrace& operator=(const StackTrace&) = delete;
        StackTrace(StackTrace&&)                 = delete;
        StackTrace& operator=(StackTrace&&)      = delete;

        

        bool pushFrame(const Records::LogEntry& ro_Entry) noexcept {
            Frame* node = m_allocator.template allocate<Frame>(1);
            if (STRATUM_UNLIKELY(!node)) return false;
            ::new(node) Frame{};
            node->m_frame.emplace(ro_Entry);
            m_tail->m_next = node;
            m_tail = node;
            return true;
        }

        bool pushFrame(const Records::ExceptionEntry& ro_Entry) noexcept {
            Frame* node = m_allocator.template allocate<Frame>(1);
            if (STRATUM_UNLIKELY(!node)) return false;
            ::new(node) Frame{};
            node->m_frame.emplace(ro_Entry);
            m_tail->m_next = node;
            m_tail = node;
            return true;
        }

        bool pushFrame(const Records::TracerEntry& ro_Entry) noexcept {
            Frame* node = m_allocator.template allocate<Frame>(1);
            if (STRATUM_UNLIKELY(!node)) return false;
            ::new(node) Frame{};
            node->m_frame.emplace(ro_Entry);
            m_tail->m_next = node;
            m_tail = node;
            return true;
        }

        

        template<typename T>
        STRATUM_NODISCARD T popFrame() noexcept {
            if (STRATUM_UNLIKELY(!m_sentinel->m_next))
                return T::s_invalid;

            Frame* node  = m_sentinel->m_next;
            T*     entry = node->m_frame.template get<T>();

            STRATUM_ASSERT(entry != nullptr);
            T result = *entry;

            if (m_tail == node) m_tail = m_sentinel;
            m_sentinel->m_next = node->m_next;

            node->m_frame.destroy();
            m_allocator.deallocate(node);
            return result;
        }

        

        STRATUM_NODISCARD Profiler::EntryKind peekFrame() const noexcept {
            if (!m_sentinel->m_next) return Profiler::EntryKind::None;
            return m_sentinel->m_next->m_frame.kind();
        }

        STRATUM_NODISCARD bool empty() const noexcept {
            return m_sentinel->m_next == nullptr;
        }

        

        ~StackTrace() noexcept {
            Frame* current = m_sentinel;
            while (current) {
                Frame* next = current->m_next;
                if (current != m_sentinel)
                    current->m_frame.destroy();
                m_allocator.deallocate(current);
                current = next;
            }
            m_sentinel = nullptr;
            m_tail     = nullptr;
        }
    };

}

namespace Stratum::Tracing::this_thread {
    inline thread_local StackTrace<> t_Trace;
}
