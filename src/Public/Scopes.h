#pragma once
#include "Stratum.h"
#include "Records.h"

namespace Stratum::Tracing {

    
    
    

    template<typename T, typename S, typename E>
    struct Scope final {
    private:
        S  m_startFn;
        E  m_endFn;
        T& m_traceable;

    public:
        Scope(T& r_TraceObject, S&& u_StartFn, E&& u_EndFn) noexcept
            : m_startFn(std::move(u_StartFn))
            , m_endFn(std::move(u_EndFn))
            , m_traceable(r_TraceObject)
        {
            if (m_startFn) m_startFn(m_traceable);
        }

        ~Scope() noexcept {
            if (m_endFn) m_endFn(m_traceable);
        }

        Scope(const Scope&)            = delete;
        Scope& operator=(const Scope&) = delete;
        Scope(Scope&&)                 = delete;
        Scope& operator=(Scope&&)      = delete;
    };

    
    
    

    template<typename F>
    struct ExceptionScope final {
    private:
        F           m_fn;
        const char* m_component;

    public:
        static_assert(
            std::is_invocable_r_v<Records::ExceptionEntry, F, const char*>,
            "ExceptionScope lambda must have signature: "
            "(const char* p_Component) -> Records::ExceptionEntry"
        );

        ExceptionScope(const char* p_Component, F&& u_Fn) noexcept
            : m_fn(std::move(u_Fn))
            , m_component(p_Component)
        {}

        Records::ExceptionEntry run() noexcept {
            try {
                return m_fn(m_component);
            }
            catch (const std::exception& r_Exc) {
                return Records::ExceptionEntry{ m_component, r_Exc.what() };
            }
            catch (...) {
                return Records::ExceptionEntry{ m_component, "Unknown fatal error" };
            }
        }
    };

} 

#ifndef STRATUM_TRACE
#define STRATUM_TRACE(v_TracerName, p_Component, p_Label)                           \
    Stratum::Records::TracerEntry v_TracerName{ p_Component, p_Label };             \
    {                                                                               \
        auto l_scope = Stratum::Tracing::Scope{                                     \
            v_TracerName,                                                            \
            [](Stratum::Records::TracerEntry& r_T) noexcept { r_T.start(); },       \
            [](Stratum::Records::TracerEntry& r_T) noexcept { r_T.end();   }        \
        };                                                                           \
        (void)l_scope;
#endif

#ifndef STRATUM_TRACE_END
#define STRATUM_TRACE_END }
#endif

#ifndef STRATUM_TRY
#define STRATUM_TRY(p_Component)                                                    \
    {                                                                               \
        auto l_exScope = Stratum::Tracing::ExceptionScope{                         \
            p_Component,                                                            \
            [](const char* p_Comp) -> Stratum::Records::ExceptionEntry
#endif

#ifndef STRATUM_CATCH
#define STRATUM_CATCH(v_ProfileName)                                                \
        };                                                                          \
        Stratum::Records::ExceptionEntry v_ProfileName = l_exScope.run();          \
    
#endif

#ifndef STRATUM_THROW
#define STRATUM_THROW(p_Message)                                                    \
    return Stratum::Records::ExceptionEntry{ p_Comp, p_Message }
#endif
