#include "PeterWorkshop/WorkshopTuning.h"

#include <sstream>
#include <set>

namespace Peter::Workshop
{
  namespace
  {
    struct PreviewValidationResult
    {
      bool valid = false;
      std::string message;
    };

    PreviewValidationResult ValidatePreviewConfig(const Peter::AI::CompanionConfig& config)
    {
      if (Peter::AI::FindBehaviorStance(config.stanceId) == nullptr)
      {
        return PreviewValidationResult{false, "Companion stance must reference a known behavior stance."};
      }

      std::set<std::string, std::less<>> uniqueChips;
      if (config.activeChipIds.size() > 3)
      {
        return PreviewValidationResult{false, "Only three active behavior chips are allowed at once."};
      }

      for (const auto& chipId : config.activeChipIds)
      {
        if (!uniqueChips.insert(chipId).second)
        {
          return PreviewValidationResult{false, "Behavior chips cannot be duplicated."};
        }
        if (Peter::AI::FindBehaviorChip(chipId) == nullptr)
        {
          return PreviewValidationResult{false, "Behavior chips must reference the known chip catalog."};
        }
      }

      const auto followDistance = Peter::AI::ResolveFollowDistance(config);
      if (followDistance < 4.0 || followDistance > 10.0)
      {
        return PreviewValidationResult{false, "Follow distance must stay between 4 and 10 meters."};
      }

      return PreviewValidationResult{true, "Companion behavior preview is safe to apply."};
    }

    PreviewValidationResult ValidatePreviewFollowDistance(const double followDistance)
    {
      if (followDistance < 4.0)
      {
        return PreviewValidationResult{false, "Follow distance cannot go below 4 meters."};
      }
      if (followDistance > 10.0)
      {
        return PreviewValidationResult{false, "Follow distance cannot exceed 10 meters."};
      }
      return PreviewValidationResult{true, "Follow distance edit is safe to apply."};
    }
  } // namespace

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
    const auto validation = ValidatePreviewConfig(previewConfig);

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
    const auto validation = ValidatePreviewFollowDistance(previewValue);

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
