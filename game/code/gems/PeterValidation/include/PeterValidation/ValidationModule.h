#pragma once

#include "PeterAdapters/PlatformServices.h"
#include "PeterInventory/InventoryState.h"
#include "PeterUI/SlicePresentation.h"
#include "PeterWorld/SliceContent.h"

#include <string>
#include <string_view>
#include <vector>

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
  [[nodiscard]] RuleValidationResult ValidateItemDefinition(const Peter::Inventory::ItemDefinition& definition);
  [[nodiscard]] RuleValidationResult ValidateMissionTemplate(const Peter::World::MissionTemplateDefinition& mission);
  [[nodiscard]] RuleValidationResult ValidateTutorialLesson(const Peter::World::TutorialLessonDefinition& lesson);
  [[nodiscard]] RuleValidationResult ValidateFavoriteItemReference(
    std::string_view favoriteItemId,
    const Peter::Inventory::InventoryState& inventory);
  [[nodiscard]] RuleValidationResult ValidateInputBindings(
    const std::vector<Peter::Adapters::ActionBinding>& bindings,
    const Peter::UI::AccessibilitySettings& settings);
  std::string_view GetModuleSummary();
} // namespace Peter::Validation
