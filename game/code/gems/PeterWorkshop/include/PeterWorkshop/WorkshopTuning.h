#pragma once

#include "PeterAI/CompanionAi.h"

#include <map>
#include <string>
#include <string_view>
#include <vector>

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
    std::string comparisonSummary;
    std::string previewLabel;
  };

  [[nodiscard]] Peter::AI::CompanionConfig BuildBehaviorPreviewConfig(
    const Peter::AI::CompanionConfig& currentConfig,
    std::string_view stanceId,
    const std::vector<std::string>& chipIds,
    const std::map<std::string, double, std::less<>>& chipValues);
  [[nodiscard]] RuleEditPreview BuildCompanionBehaviorPreview(
    const Peter::AI::CompanionConfig& currentConfig,
    const Peter::AI::CompanionConfig& previewConfig);
  [[nodiscard]] RuleEditPreview BuildFollowDistancePreview(double currentValue, double previewValue);
} // namespace Peter::Workshop
