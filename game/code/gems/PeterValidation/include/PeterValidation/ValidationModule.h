#pragma once

#include <string>
#include <string_view>

namespace Peter::Validation
{
  struct ValidationStatus
  {
    std::string status;
    std::string summary;

    [[nodiscard]] static ValidationStatus PlaceholderHealthy();
  };

  std::string_view GetModuleSummary();
} // namespace Peter::Validation
