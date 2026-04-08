#pragma once

#include <string>

namespace Peter::Workshop
{
  struct RuleEditPreview
  {
    std::string ruleId;
    double currentValue = 0.0;
    double previewValue = 0.0;
    bool valid = false;
    std::string summary;
    std::string deltaSummary;
  };

  [[nodiscard]] RuleEditPreview BuildFollowDistancePreview(double currentValue, double previewValue);
} // namespace Peter::Workshop
