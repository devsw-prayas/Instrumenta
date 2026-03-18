#pragma once
#include "Stratum.h"
#include "Records.h"

namespace Stratum::Filtering {

    

    enum class FilterMode : uint8_t {
        Allowlist,  
        Denylist    
    };

    
    
    
    
    
    
    
    
    

    class STRATUM_API FilterChain final {
    private:
        static constexpr size_t MAX_FILTER_ENTRIES = 16;

        Records::LogLevel   m_minLevel          = Records::LogLevel::Info;

        char    m_componentEntries[MAX_FILTER_ENTRIES][Records::COMPONENT_NAME_MAX]{};
        size_t  m_componentCount                = 0;
        FilterMode m_componentMode              = FilterMode::Allowlist;
        bool    m_componentFilterActive         = false;

        char    m_tagEntries[MAX_FILTER_ENTRIES][Records::TAG_MAX]{};
        size_t  m_tagCount                      = 0;
        bool    m_tagFilterActive               = false;

    public:
        FilterChain() noexcept = default;

        

        void setMinLevel(Records::LogLevel v_MinLevel) noexcept;

        

        void setComponentMode(FilterMode v_Mode)          noexcept;
        bool addComponentFilter(const char* p_Component)  noexcept;
        void clearComponentFilters()                       noexcept;

        

        bool addTagFilter(const char* p_Tag) noexcept;
        void clearTagFilters()               noexcept;

        

        STRATUM_NODISCARD STRATUM_FORCEINLINE
        bool accepts(
            Records::LogLevel   v_Level,
            const char*         p_Component,
            const char*         p_Tag
        ) const noexcept {

            
            if (STRATUM_UNLIKELY(static_cast<uint8_t>(v_Level) <
                                 static_cast<uint8_t>(m_minLevel)))
                return false;

            
            if (m_componentFilterActive && p_Component) {
                bool found = false;
                for (size_t i = 0; i < m_componentCount; ++i) {
                    if (std::strncmp(m_componentEntries[i], p_Component,
                                     Records::COMPONENT_NAME_MAX) == 0) {
                        found = true;
                        break;
                    }
                }
                if (m_componentMode == FilterMode::Allowlist && !found) return false;
                if (m_componentMode == FilterMode::Denylist  &&  found) return false;
            }

            
            if (m_tagFilterActive && p_Tag) {
                bool found = false;
                for (size_t i = 0; i < m_tagCount; ++i) {
                    if (std::strncmp(m_tagEntries[i], p_Tag,
                                     Records::TAG_MAX) == 0) {
                        found = true;
                        break;
                    }
                }
                if (!found) return false;
            }

            return true;
        }
    };

}
