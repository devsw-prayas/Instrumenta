#pragma once
#include "Stratum.h"
#include "StratumMemory.h"
#include "StratumTraits.h"

namespace Stratum::Tracing {
	using StackFrame = Traits::ProfileRecord;

	struct Frame {
		Frame* m_NextFrame;
		StackFrame m_Frame;
	};

	template<typename A = Memory::MemoryAllocator>
	class STRATUM StackTrace final {
		using Allocator = A;
		Frame* m_Sentinel;
		Frame* m_Tail;
		Allocator m_FrameAllocator;
	public:
		StackTrace() : m_Tail(nullptr), m_FrameAllocator() {
			m_Sentinel = m_FrameAllocator.template allocate<Frame>(1);
			m_Sentinel->m_NextFrame = nullptr;
			m_Tail = m_Sentinel;
		}

		bool pushFrame(const ExceptionProfile& ro_ExceptionProfile) {
			m_Tail->m_NextFrame = m_FrameAllocator.template allocate<Frame>(1);
			if (!m_Tail->m_NextFrame) return false;
			m_Tail = m_Tail->m_NextFrame;
			m_Tail->m_Frame.emplace(ro_ExceptionProfile);
			return true;
		}

		bool pushFrame(const TracingProfile& ro_TraceProfile) {
			m_Tail->m_NextFrame = m_FrameAllocator.template allocate<Frame>(1);
			if (!m_Tail->m_NextFrame) return false;
			m_Tail = m_Tail->m_NextFrame;
			m_Tail->m_Frame.emplace(ro_TraceProfile);
			return true;
		}

		bool pushFrame(const LogEntry& ro_LogEntry) {
			m_Tail->m_NextFrame = m_FrameAllocator.template allocate<Frame>(1);
			if (!m_Tail->m_NextFrame) return false;
			m_Tail = m_Tail->m_NextFrame;
			m_Tail->m_Frame.emplace(ro_LogEntry);
			return true;
		}

		template<typename T>
		T popFrame() {
			if (!m_Sentinel->m_NextFrame) return T::s_Invalid;
			Frame* frame = m_Sentinel->m_NextFrame;
			T entry = frame->m_Frame.get<T>();
			if (m_Tail == frame) m_Tail = m_Sentinel;
			m_Sentinel->m_NextFrame = frame->m_NextFrame;
			m_FrameAllocator.destroy(&frame->m_Frame);
			m_FrameAllocator.deallocate(frame);
			return entry;
		}

		Traits::RecordType peekFrame() const {
			if (!m_Sentinel->m_NextFrame) return Traits::RecordType::NONE;
			return m_Sentinel->m_NextFrame->m_Frame.kind();
		}
	};
}
