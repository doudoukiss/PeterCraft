#pragma once

#include "PeterAI/CompanionAi.h"
#include "PeterAdapters/PlatformServices.h"
#include "PeterInventory/InventoryState.h"
#include "PeterUI/SlicePresentation.h"
#include "PeterWorkshop/CreatorWorkshop.h"
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
  [[nodiscard]] RuleValidationResult ValidateCompanionConfig(const Peter::AI::CompanionConfig& config);
  [[nodiscard]] RuleValidationResult ValidateBehaviorChipDefinition(const Peter::AI::BehaviorChipDefinition& chip);
  [[nodiscard]] RuleValidationResult ValidateBehaviorStanceDefinition(const Peter::AI::BehaviorStanceDefinition& stance);
  [[nodiscard]] RuleValidationResult ValidateEnemyArchetypeDefinition(
    const Peter::AI::EnemyArchetypeDefinition& archetype);
  [[nodiscard]] RuleValidationResult ValidatePatrolRouteDefinition(const Peter::AI::PatrolRouteDefinition& route);
  [[nodiscard]] RuleValidationResult ValidateAiScenarioDefinition(const Peter::AI::AiScenarioDefinition& scenario);
  [[nodiscard]] RuleValidationResult ValidateTinkerVariableDefinition(
    const Peter::Workshop::TinkerVariableDefinition& variable);
  [[nodiscard]] RuleValidationResult ValidateTinkerPresetDefinition(
    const Peter::Workshop::TinkerPresetDefinition& preset);
  [[nodiscard]] RuleValidationResult ValidateLogicRulesetDefinition(
    const Peter::Workshop::LogicRulesetDefinition& ruleset);
  [[nodiscard]] RuleValidationResult ValidateTinyScriptDefinition(
    const Peter::Workshop::TinyScriptDefinition& script);
  [[nodiscard]] RuleValidationResult ValidateMiniMissionDraftDefinition(
    const Peter::Workshop::MiniMissionDraftDefinition& draft);
  [[nodiscard]] RuleValidationResult ValidateCreatorManifest(
    const Peter::Workshop::CreatorManifest& manifest);
  [[nodiscard]] RuleValidationResult ValidateFavoriteItemReference(
    std::string_view favoriteItemId,
    const Peter::Inventory::InventoryState& inventory);
  [[nodiscard]] RuleValidationResult ValidateInputBindings(
    const std::vector<Peter::Adapters::ActionBinding>& bindings,
    const Peter::UI::AccessibilitySettings& settings);
  std::string_view GetModuleSummary();
} // namespace Peter::Validation
