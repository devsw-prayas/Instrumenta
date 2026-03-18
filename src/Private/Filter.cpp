#include "../Public/Stratum.h"
#include "../Public/Filter.h"

namespace Stratum::Filtering {

    void FilterChain::setMinLevel(Records::LogLevel v_MinLevel) noexcept {
        m_minLevel = v_MinLevel;
    }

    void FilterChain::setComponentMode(FilterMode v_Mode) noexcept {
        m_componentMode = v_Mode;
    }

    bool FilterChain::addComponentFilter(const char* p_Component) noexcept {
        if (STRATUM_UNLIKELY(!p_Component))                           return false;
        if (STRATUM_UNLIKELY(m_componentCount >= MAX_FILTER_ENTRIES)) return false;
        std::strncpy(m_componentEntries[m_componentCount], p_Component, Records::COMPONENT_NAME_MAX - 1);
        m_componentEntries[m_componentCount][Records::COMPONENT_NAME_MAX - 1] = '\0';
        ++m_componentCount;
        m_componentFilterActive = true;
        return true;
    }

    void FilterChain::clearComponentFilters() noexcept {
        m_componentCount        = 0;
        m_componentFilterActive = false;
    }

    bool FilterChain::addTagFilter(const char* p_Tag) noexcept {
        if (STRATUM_UNLIKELY(!p_Tag))                          return false;
        if (STRATUM_UNLIKELY(m_tagCount >= MAX_FILTER_ENTRIES)) return false;
        std::strncpy(m_tagEntries[m_tagCount], p_Tag, Records::TAG_MAX - 1);
        m_tagEntries[m_tagCount][Records::TAG_MAX - 1] = '\0';
        ++m_tagCount;
        m_tagFilterActive = true;
        return true;
    }

    void FilterChain::clearTagFilters() noexcept {
        m_tagCount        = 0;
        m_tagFilterActive = false;
    }

}
