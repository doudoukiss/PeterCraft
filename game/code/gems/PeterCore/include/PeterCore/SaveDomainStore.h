#pragma once

#include "PeterCore/EventBus.h"
#include "PeterCore/ProfileService.h"

#include <filesystem>
#include <string>
#include <string_view>

namespace Peter::Core
{
  class SaveDomainStore
  {
  public:
    SaveDomainStore(ProfileInfo profile, EventBus& eventBus);

    [[nodiscard]] bool DomainExists(std::string_view domainId) const;
    [[nodiscard]] StructuredFields ReadDomain(const std::string& domainId) const;
    void WriteDomain(const std::string& domainId, const StructuredFields& fields) const;

  private:
    [[nodiscard]] std::filesystem::path DomainPath(std::string_view domainId) const;

    ProfileInfo m_profile;
    EventBus& m_eventBus;
  };
} // namespace Peter::Core
