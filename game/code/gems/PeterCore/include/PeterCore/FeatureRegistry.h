#pragma once

#include <map>
#include <string>
#include <string_view>

namespace Peter::Core
{
  struct VersionInfo
  {
    std::string semanticVersion;
    std::string track;
  };

  class FeatureRegistry
  {
  public:
    explicit FeatureRegistry(VersionInfo versionInfo);

    void SetFlag(const std::string& flagName, bool enabled);
    [[nodiscard]] bool IsEnabled(std::string_view flagName) const;
    [[nodiscard]] const std::map<std::string, bool, std::less<>>& Flags() const;
    [[nodiscard]] const VersionInfo& Version() const;

  private:
    VersionInfo m_versionInfo;
    std::map<std::string, bool, std::less<>> m_flags;
  };
} // namespace Peter::Core
