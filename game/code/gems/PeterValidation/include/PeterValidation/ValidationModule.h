#pragma once

#include <string>
#include <string_view>

namespace Peter::Validation
{
  struct RuleValidationResult
  {
    bool valid = false;
    std::string message;
  };

  struct ValidationStatus
  {
    std::string status;
    std::string summary;

    [[nodiscard]] static ValidationStatus PlaceholderHealthy();
  };

  [[nodiscard]] RuleValidationResult ValidateFollowDistance(double followDistanceMeters);
  [[nodiscard]] RuleValidationResult ValidateExtractionCountdown(int countdownSeconds);
  std::string_view GetModuleSummary();
} // namespace Peter::Validation
