#pragma once

#include "PeterCore/EventBus.h"
#include "PeterCore/ProfileService.h"

#include <cstddef>
#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace Peter::Core
{
  struct SaveReadResult
  {
    bool valid = false;
    bool restoredFromBackup = false;
    StructuredFields fields;
    std::string message;
    std::filesystem::path sourcePath;
  };

  struct SaveWriteResult
  {
    bool success = false;
    double durationMs = 0.0;
    std::size_t bytesWritten = 0;
    std::string message;
    std::filesystem::path path;
    std::filesystem::path latestBackupPath;
    std::filesystem::path previousBackupPath;
  };

  struct SaveRestoreResult
  {
    bool success = false;
    std::string message;
    std::filesystem::path restoredFromPath;
    std::filesystem::path restoredToPath;
  };

  struct SaveHealthReport
  {
    bool healthy = true;
    int checkedDomains = 0;
    int restoredDomains = 0;
    std::vector<std::string> invalidDomainIds;
    std::string summary;
  };

  class SaveDomainStore
  {
  public:
    SaveDomainStore(ProfileInfo profile, EventBus& eventBus);

    [[nodiscard]] bool DomainExists(std::string_view domainId) const;
    [[nodiscard]] SaveReadResult ReadDomainChecked(const std::string& domainId) const;
    [[nodiscard]] StructuredFields ReadDomain(const std::string& domainId) const;
    [[nodiscard]] SaveWriteResult WriteDomain(const std::string& domainId, const StructuredFields& fields) const;
    [[nodiscard]] SaveRestoreResult RestoreDomain(std::string_view domainId) const;
    [[nodiscard]] SaveHealthReport InspectHealth() const;

  private:
    [[nodiscard]] std::filesystem::path DomainPath(std::string_view domainId) const;
    [[nodiscard]] std::filesystem::path LatestBackupPath(std::string_view domainId) const;
    [[nodiscard]] std::filesystem::path PreviousBackupPath(std::string_view domainId) const;

    ProfileInfo m_profile;
    EventBus& m_eventBus;
  };
} // namespace Peter::Core
