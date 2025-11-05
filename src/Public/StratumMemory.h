#pragma once
#include "Stratum.h"

namespace Stratum::Memory {
	class STRATUM MemoryAllocator final {
	public:
		template<typename T>
		T* allocate(size_t v_Elements) const {
			return static_cast<T*>(malloc(sizeof(T) * v_Elements));
		}

		void deallocate(void* p_Memory) const {
			free(p_Memory);
		}

		template<typename T, typename... Args>
		void construct(T* p_Object, Args&&...u_Args) {
			::new(p_Object) T(std::forward<Args>(u_Args)...);
		}

		template<typename T>
		void destroy(T* p_Object) {
			p_Object->~T();
		}
	};
}