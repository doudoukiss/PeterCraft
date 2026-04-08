#include "PeterValidation/ValidationModule.h"

#include <set>

namespace Peter::Validation
{
  ValidationStatus ValidationStatus::PlaceholderHealthy()
  {
    return ValidationStatus{"ok", "phase 3 validation, AI authoring checks, migrations, and deterministic scenario guards are active"};
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

  RuleValidationResult ValidateCompanionConfig(const Peter::AI::CompanionConfig& config)
  {
    if (Peter::AI::FindBehaviorStance(config.stanceId) == nullptr)
    {
      return RuleValidationResult{false, "Companion stance must reference a known Phase 3 stance."};
    }

    std::set<std::string, std::less<>> uniqueChips;
    if (config.activeChipIds.size() > 3)
    {
      return RuleValidationResult{false, "Only three active behavior chips are allowed at once."};
    }

    for (const auto& chipId : config.activeChipIds)
    {
      if (!uniqueChips.insert(chipId).second)
      {
        return RuleValidationResult{false, "Behavior chips cannot be duplicated."};
      }
      if (Peter::AI::FindBehaviorChip(chipId) == nullptr)
      {
        return RuleValidationResult{false, "Behavior chips must reference the Phase 3 chip catalog."};
      }
    }

    return ValidateFollowDistance(Peter::AI::ResolveFollowDistance(config));
  }

  RuleValidationResult ValidateBehaviorChipDefinition(const Peter::AI::BehaviorChipDefinition& chip)
  {
    if (chip.id.empty() || chip.displayName.empty() || chip.promiseText.empty() || chip.mappedVariableId.empty())
    {
      return RuleValidationResult{false, "Behavior chips need id, display name, promise text, and a mapped variable."};
    }

    if (chip.minValue > chip.maxValue || chip.defaultValue < chip.minValue || chip.defaultValue > chip.maxValue)
    {
      return RuleValidationResult{false, "Behavior chip ranges must be safe and internally consistent."};
    }

    if (chip.id == "chip.stay_near_me" && (chip.minValue < 4.0 || chip.maxValue > 10.0))
    {
      return RuleValidationResult{false, "Stay Near Me must stay within the safe 4m to 10m range."};
    }

    return RuleValidationResult{true, "Behavior chip definition is valid."};
  }

  RuleValidationResult ValidateBehaviorStanceDefinition(const Peter::AI::BehaviorStanceDefinition& stance)
  {
    if (stance.id.empty() || stance.displayName.empty() || stance.summary.empty())
    {
      return RuleValidationResult{false, "Behavior stances need id, display name, and summary text."};
    }

    return RuleValidationResult{true, "Behavior stance definition is valid."};
  }

  RuleValidationResult ValidateEnemyArchetypeDefinition(
    const Peter::AI::EnemyArchetypeDefinition& archetype)
  {
    if (archetype.id.empty() || archetype.displayName.empty() || archetype.summary.empty())
    {
      return RuleValidationResult{false, "Enemy archetypes need id, display name, and summary text."};
    }

    if (archetype.preferredRangeMeters < 1)
    {
      return RuleValidationResult{false, "Enemy archetypes need a positive preferred range."};
    }

    return RuleValidationResult{true, "Enemy archetype definition is valid."};
  }

  RuleValidationResult ValidatePatrolRouteDefinition(const Peter::AI::PatrolRouteDefinition& route)
  {
    if (route.id.empty() || route.nodeIds.size() < 2)
    {
      return RuleValidationResult{false, "Patrol routes need an id and at least two linked nodes."};
    }

    std::set<std::string, std::less<>> uniqueNodes;
    for (const auto& nodeId : route.nodeIds)
    {
      if (!uniqueNodes.insert(nodeId).second)
      {
        return RuleValidationResult{false, "Patrol routes cannot repeat node ids in this shell."};
      }
    }

    return RuleValidationResult{true, "Patrol route definition is valid."};
  }

  RuleValidationResult ValidateAiScenarioDefinition(const Peter::AI::AiScenarioDefinition& scenario)
  {
    if (scenario.id.empty() || scenario.steps.empty())
    {
      return RuleValidationResult{false, "AI scenarios need a stable id and at least one step."};
    }

    if (!scenario.deterministic)
    {
      return RuleValidationResult{false, "Phase 3 AI scenarios must be deterministic."};
    }

    for (const auto& step : scenario.steps)
    {
      if (step.expectedActionId.empty() || step.expectedState.empty())
      {
        return RuleValidationResult{false, "Every AI scenario step needs an expected action and state."};
      }
      if (step.useEnemy && step.enemy.enemyId.empty())
      {
        return RuleValidationResult{false, "Enemy AI scenario steps need an enemy unit."};
      }
      if (!step.useEnemy)
      {
        const auto configValidation = ValidateCompanionConfig(step.config);
        if (!configValidation.valid)
        {
          return configValidation;
        }
      }
    }

    return RuleValidationResult{true, "AI scenario definition is valid."};
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
    return "Runtime validation hooks, save migrations, mission checks, and authored safety boundaries including AI contracts.";
  }
} // namespace Peter::Validation
