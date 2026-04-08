#pragma once

#include "PeterAdapters/PlatformServices.h"
#include "PeterCore/EventBus.h"

#include <filesystem>
#include <string>

namespace Peter::Core
{
  struct ProfileInfo
  {
    std::string profileId;
    std::filesystem::path root;
    std::filesystem::path saveDataRoot;
    std::filesystem::path userContentRoot;
  };

  class ProfileService
  {
  public:
    ProfileService(const Peter::Adapters::ISaveAdapter& saveAdapter, EventBus& eventBus);
    [[nodiscard]] ProfileInfo EnsureProfile(const std::string& profileId) const;

  private:
    const Peter::Adapters::ISaveAdapter& m_saveAdapter;
    EventBus& m_eventBus;
  };
} // namespace Peter::Core
