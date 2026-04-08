#include "PeterWorkshop/WorkshopTuning.h"

#include "PeterValidation/ValidationModule.h"

#include <sstream>

namespace Peter::Workshop
{
  Peter::AI::CompanionConfig BuildBehaviorPreviewConfig(
    const Peter::AI::CompanionConfig& currentConfig,
    const std::string_view stanceId,
    const std::vector<std::string>& chipIds,
    const std::map<std::string, double, std::less<>>& chipValues)
  {
    Peter::AI::CompanionConfig preview = currentConfig;
    preview.stanceId = std::string(stanceId);
    preview.activeChipIds = chipIds;
    for (const auto& [chipId, value] : chipValues)
    {
      preview.chipValues[chipId] = value;
    }
    preview.followDistanceMeters = Peter::AI::ResolveFollowDistance(preview);
    return preview;
  }

  RuleEditPreview BuildCompanionBehaviorPreview(
    const Peter::AI::CompanionConfig& currentConfig,
    const Peter::AI::CompanionConfig& previewConfig)
  {
    const auto validation = Peter::Validation::ValidateCompanionConfig(previewConfig);

    std::ostringstream summary;
    summary << "Companion behavior preview: " << previewConfig.stanceId
            << " with " << previewConfig.activeChipIds.size() << " chips";

    std::ostringstream delta;
    delta << "Follow distance shifts from " << Peter::AI::ResolveFollowDistance(currentConfig)
          << "m to " << Peter::AI::ResolveFollowDistance(previewConfig) << "m.";
    if (!validation.valid)
    {
      delta << " " << validation.message;
    }

    std::ostringstream comparison;
    comparison << "Before stance=" << currentConfig.stanceId
               << " | After stance=" << previewConfig.stanceId;

    return RuleEditPreview{
      "companion_rule.behavior_catalog.v1",
      Peter::AI::ResolveFollowDistance(currentConfig),
      Peter::AI::ResolveFollowDistance(previewConfig),
      validation.valid,
      summary.str(),
      delta.str(),
      comparison.str(),
      previewConfig.stanceId};
  }

  RuleEditPreview BuildFollowDistancePreview(const double currentValue, const double previewValue)
  {
    const auto validation = Peter::Validation::ValidateFollowDistance(previewValue);

    std::ostringstream summary;
    summary << "Follow distance preview: " << previewValue << "m";

    std::ostringstream delta;
    delta << (previewValue > currentValue ? "Companion will give the player more space."
                                          : "Companion will stay closer to the player.");

    if (!validation.valid)
    {
      delta << " " << validation.message;
    }

    std::ostringstream comparison;
    comparison << "Before " << currentValue << "m | After " << previewValue << "m";

    return RuleEditPreview{
      "companion_rule.follow_distance.default",
      currentValue,
      previewValue,
      validation.valid,
      summary.str(),
      delta.str(),
      comparison.str(),
      "chip.stay_near_me"};
  }
} // namespace Peter::Workshop
