#include "PeterValidation/ValidationModule.h"

namespace Peter::Validation
{
  ValidationStatus ValidationStatus::PlaceholderHealthy()
  {
    return ValidationStatus{"ok", "schema validation and safe tuning checks are active for the Phase 1 slice"};
  }

  RuleValidationResult ValidateFollowDistance(const double followDistanceMeters)
  {
    if (followDistanceMeters < 4.0)
    {
      return RuleValidationResult{false, "Follow distance cannot go below 4 meters."};
    }

    if (followDistanceMeters > 10.0)
    {
      return RuleValidationResult{false, "Follow distance cannot exceed 10 meters."};
    }

    return RuleValidationResult{true, "Follow distance edit is safe to apply."};
  }

  RuleValidationResult ValidateExtractionCountdown(const int countdownSeconds)
  {
    if (countdownSeconds < 3 || countdownSeconds > 10)
    {
      return RuleValidationResult{false, "Extraction countdown must stay between 3 and 10 seconds."};
    }

    return RuleValidationResult{true, "Extraction countdown is valid."};
  }

  std::string_view GetModuleSummary()
  {
    return "Runtime validation hooks, safe tuning checks, and creator safety boundaries.";
  }
} // namespace Peter::Validation
