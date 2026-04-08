#include "PeterValidation/ValidationModule.h"

#include <set>

namespace Peter::Validation
{
  ValidationStatus ValidationStatus::PlaceholderHealthy()
  {
    return ValidationStatus{"ok", "phase 2 validation, migration checks, and authored content guards are active"};
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
    if (countdownSeconds < 3 || countdownSeconds > 12)
    {
      return RuleValidationResult{false, "Extraction countdown must stay between 3 and 12 seconds."};
    }

    return RuleValidationResult{true, "Extraction countdown is valid."};
  }

  RuleValidationResult ValidateItemDefinition(const Peter::Inventory::ItemDefinition& definition)
  {
    if (definition.id.empty() || definition.displayName.empty() || definition.description.empty())
    {
      return RuleValidationResult{false, "Item definitions must include id, display name, and description."};
    }

    if (definition.stackLimit < 1)
    {
      return RuleValidationResult{false, "Item stack limits must be positive."};
    }

    if (!Peter::Inventory::IsRarityAllowed(definition.category, definition.rarity))
    {
      return RuleValidationResult{false, "That rarity is not allowed for the chosen category."};
    }

    if (Peter::Inventory::IsDurableCategory(definition.category) && !definition.repairable)
    {
      return RuleValidationResult{false, "Durable categories must be marked repairable."};
    }

    return RuleValidationResult{true, "Item definition is valid."};
  }

  RuleValidationResult ValidateMissionTemplate(const Peter::World::MissionTemplateDefinition& mission)
  {
    if (mission.roomIds.size() < 2)
    {
      return RuleValidationResult{false, "Mission templates need at least two rooms."};
    }

    if (mission.objectives.empty())
    {
      return RuleValidationResult{false, "Mission templates need at least one required objective."};
    }

    std::set<std::string, std::less<>> objectiveIds;
    for (const auto& objective : mission.objectives)
    {
      if (!objectiveIds.insert(objective.id).second)
      {
        return RuleValidationResult{false, "Mission objective IDs must be unique."};
      }
      if (objective.description.empty() || objective.kind.empty())
      {
        return RuleValidationResult{false, "Mission objectives need descriptions and kinds."};
      }
    }

    for (const auto& sideObjective : mission.sideObjectives)
    {
      if (!objectiveIds.insert(sideObjective.id).second)
      {
        return RuleValidationResult{false, "Side-objective IDs must not collide with required objectives."};
      }
    }

    return RuleValidationResult{true, "Mission template is valid."};
  }

  RuleValidationResult ValidateTutorialLesson(const Peter::World::TutorialLessonDefinition& lesson)
  {
    if (lesson.steps.empty())
    {
      return RuleValidationResult{false, "Lessons need at least one step."};
    }

    bool hasCompletion = false;
    for (const auto& step : lesson.steps)
    {
      if (step.prompt.empty() || step.action.empty())
      {
        return RuleValidationResult{false, "Every lesson step needs an action and prompt."};
      }
      if (step.action == "complete_lesson")
      {
        hasCompletion = true;
      }
    }

    if (!hasCompletion)
    {
      return RuleValidationResult{false, "Lessons must include a complete_lesson step."};
    }

    return RuleValidationResult{true, "Tutorial lesson is valid."};
  }

  RuleValidationResult ValidateFavoriteItemReference(
    const std::string_view favoriteItemId,
    const Peter::Inventory::InventoryState& inventory)
  {
    if (favoriteItemId.empty())
    {
      return RuleValidationResult{true, "No favorite item selected."};
    }

    if (Peter::Inventory::CountItem(inventory.equippedDurability, favoriteItemId) == 0)
    {
      return RuleValidationResult{false, "Favorite item must reference equipped durable gear."};
    }

    return RuleValidationResult{true, "Favorite item reference is valid."};
  }

  RuleValidationResult ValidateInputBindings(
    const std::vector<Peter::Adapters::ActionBinding>& bindings,
    const Peter::UI::AccessibilitySettings& settings)
  {
    std::set<std::string, std::less<>> activeBindings;
    for (const auto& binding : bindings)
    {
      const auto override = settings.actionBindings.find(binding.actionId);
      const std::string resolved = override == settings.actionBindings.end() ? binding.primaryInput : override->second;
      if (!activeBindings.insert(resolved).second)
      {
        return RuleValidationResult{false, "Two actions cannot share the same primary binding in this shell."};
      }
    }

    return RuleValidationResult{true, "Input bindings are conflict-free."};
  }

  std::string_view GetModuleSummary()
  {
    return "Runtime validation hooks, save migrations, mission checks, and authored safety boundaries.";
  }
} // namespace Peter::Validation
