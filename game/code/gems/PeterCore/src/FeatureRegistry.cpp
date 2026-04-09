#include "PeterCore/FeatureRegistry.h"

#include <utility>

namespace Peter::Core
{
  FeatureRegistry::FeatureRegistry(VersionInfo versionInfo)
    : m_versionInfo(std::move(versionInfo))
  {
  }

  void FeatureRegistry::SetFlag(const std::string& flagName, const bool enabled)
  {
    m_flags[flagName] = enabled;
  }

  bool FeatureRegistry::IsEnabled(const std::string_view flagName) const
  {
    const auto iterator = m_flags.find(std::string(flagName));
    return iterator != m_flags.end() ? iterator->second : false;
  }

  const std::map<std::string, bool, std::less<>>& FeatureRegistry::Flags() const
  {
    return m_flags;
  }

  const VersionInfo& FeatureRegistry::Version() const
  {
    return m_versionInfo;
  }
} // namespace Peter::Core
