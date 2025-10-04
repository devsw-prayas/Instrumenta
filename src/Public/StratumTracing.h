#pragma once
#include "TracerUtils.h"

namespace Stratum::Tracing {
	template<typename T, typename S, typename E>
	struct Scope final {
		S m_StartFunction;
		E m_EndFunction;
		T& m_Traceable;

		Scope(T& ro_TraceObject, S&& u_StartFunction, E&& u_EndFunction)
			:m_StartFunction(std::move(u_StartFunction)), m_EndFunction(std::move(u_EndFunction)), m_Traceable(ro_TraceObject) {
			if (m_StartFunction) m_StartFunction(m_Traceable);
		}

		~Scope() {
			if (m_EndFunction) m_EndFunction(m_Traceable);
		}

		Scope(const Scope&) = delete;
		Scope& operator=(const Scope&) = delete;
		Scope(Scope&&) noexcept = delete;
		Scope& operator=(Scope&&) noexcept = delete;
	};

#ifndef TRACE
#define TRACE(trace, component, label) \
    Stratum::Tracing::TracingProfile trace{component, label};\
	{auto scope = ::Stratum::Tracing::Scope{ trace, \
			[](Stratum::Tracing::TracingProfile& ro_Tracer) {ro_Tracer.startTrace(); },\
			[](Stratum::Tracing::TracingProfile& ro_Tracer) {ro_Tracer.endTrace(); } }; \

#endif

	template<typename F>
	struct ExceptionScope final {
		F m_ThrowableFunction;
		const char* m_Component;
		ExceptionScope(const char* p_Component, F&& ro_ThrowableFunction) :
			m_ThrowableFunction(std::move(ro_ThrowableFunction)), m_Component(p_Component) {}
		static_assert(std::is_invocable_r_v<Stratum::Tracing::ExceptionProfile, F>, "Invalid scope function! Must return ExceptionProfile");
		Stratum::Tracing::ExceptionProfile run() {
			try {
				return m_ThrowableFunction(m_Component);
			}
			catch (const std::exception& exc) {
				return Stratum::Tracing::ExceptionProfile{ m_Component, exc.what() };
			}
			catch (...) {
				return Stratum::Tracing::ExceptionProfile{ m_Component, "Fatal error occured" };
			}
		}
	};

#ifndef TRY
#define TRY(component) \
	auto scope = Stratum::Tracing::ExceptionScope{component, [](const char* p_Component) -> Stratum::Tracing::ExceptionProfile \

#endif

#ifndef CATCH
#define CATCH(profile) \
	); \
	Stratum::Tracing::ExceptionProfile profile = scope.run();
#endif

#ifndef THROW
#define THROW(message) \
	return Stratum::Tracing::ExceptionProfile{p_Component, message};
#endif

}
