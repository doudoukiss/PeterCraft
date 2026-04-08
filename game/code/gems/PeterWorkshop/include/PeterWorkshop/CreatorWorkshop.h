#pragma once

#include "PeterAI/CompanionAi.h"
#include "PeterCore/EventBus.h"

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace Peter::Workshop
{
  struct TinkerVariableDefinition
  {
    std::string id;
    std::string displayName;
    std::string summary;
    std::string controlType;
    std::string groupId;
    double defaultValue = 0.0;
    double minValue = 0.0;
    double maxValue = 1.0;
    bool creatorOnly = false;
  };

  struct TinkerPresetDefinition
  {
    std::string id;
    std::string displayName;
    std::string summary;
    std::string groupId;
    std::map<std::string, double, std::less<>> values;
  };

  struct LogicCardDefinition
  {
    std::string id;
    std::string displayName;
    std::string summary;
    std::string conditionId;
    std::string actionId;
    double scoreDelta = 0.0;
    bool gateAction = false;
  };

  struct LogicRulesetDefinition
  {
    std::string id;
    std::string displayName;
    std::string summary;
    std::vector<LogicCardDefinition> cards;
  };

  enum class TinyScriptHookKind
  {
    MissionScoreBonus,
    CompanionPriorityHint,
    TutorialMessageOverride,
    LootRarityReaction
  };

  struct TinyScriptDefinition
  {
    std::string id;
    std::string displayName;
    std::string summary;
    TinyScriptHookKind hookKind = TinyScriptHookKind::CompanionPriorityHint;
    std::string body;
    std::string targetActionId;
  };

  struct MiniMissionDraftDefinition
  {
    std::string id;
    std::string displayName;
    std::string summary;
    std::string roomBundleId;
    std::string lootGoalItemId;
    std::string enemyGroupId;
    std::string extractionPointId;
    std::string rewardBundleId;
    bool active = false;
    int revision = 1;
  };

  struct CreatorManifest
  {
    std::map<std::string, std::string, std::less<>> activeDraftIds;
    std::map<std::string, int, std::less<>> lastKnownGoodRevisions;
    std::map<std::string, bool, std::less<>> disabledContent;
    std::map<std::string, int, std::less<>> rollbackTargets;
  };

  struct CreatorProgressState
  {
    std::vector<std::string> completedCreatorLessons;
    bool mentorViewUnlocked = false;
    int safeSimulationRuns = 0;
  };

  struct CreatorSettings
  {
    bool creatorContentEnabled = true;
    bool mentorViewVisible = false;
    bool safeSimulationEnabled = true;
  };

  struct CreatorReplaySnippet
  {
    std::string scenarioId;
    std::string beforeSummary;
    std::string afterSummary;
    std::string changeSummary;
    std::vector<std::string> timeline;
  };

  struct CreatorActivationResult
  {
    bool success = false;
    std::string summary;
    int activeRevision = 0;
    int fallbackRevision = 0;
  };

  struct TinyScriptRunResult
  {
    bool valid = false;
    bool returnedString = false;
    double numericResult = 0.0;
    std::string textResult;
    int stepsUsed = 0;
    std::string error;
  };

  [[nodiscard]] constexpr int MaxMiniMissionDrafts()
  {
    return 10;
  }

  [[nodiscard]] const std::vector<TinkerVariableDefinition>& BuildPhase4TinkerVariables();
  [[nodiscard]] const TinkerVariableDefinition* FindTinkerVariable(std::string_view variableId);
  [[nodiscard]] const std::vector<TinkerPresetDefinition>& BuildPhase4TinkerPresets();
  [[nodiscard]] const TinkerPresetDefinition* FindTinkerPreset(std::string_view presetId);
  [[nodiscard]] std::map<std::string, double, std::less<>> DefaultTinkerValues();
  [[nodiscard]] Peter::AI::CompanionConfig ApplyTinkerValues(
    const Peter::AI::CompanionConfig& baseConfig,
    const std::map<std::string, double, std::less<>>& tinkerValues,
    bool creatorScoped);
  [[nodiscard]] Peter::AI::BehaviorOverrideSet BuildTinkerBehaviorOverrides(
    const std::map<std::string, double, std::less<>>& tinkerValues,
    bool creatorScoped);

  [[nodiscard]] const std::vector<LogicRulesetDefinition>& BuildPhase4LogicTemplates();
  [[nodiscard]] const LogicRulesetDefinition* FindLogicTemplate(std::string_view rulesetId);
  [[nodiscard]] Peter::AI::BehaviorOverrideSet CompileLogicRuleset(
    const LogicRulesetDefinition& ruleset,
    const Peter::AI::CompanionWorldContext& context);

  [[nodiscard]] const std::vector<TinyScriptDefinition>& BuildPhase4TinyScriptTemplates();
  [[nodiscard]] const TinyScriptDefinition* FindTinyScriptTemplate(std::string_view scriptId);
  [[nodiscard]] TinyScriptRunResult ValidateTinyScript(const TinyScriptDefinition& script);
  [[nodiscard]] TinyScriptRunResult RunTinyScript(
    const TinyScriptDefinition& script,
    const Peter::AI::CompanionWorldContext& context,
    int stepBudget = 128);
  [[nodiscard]] Peter::AI::BehaviorOverrideSet BuildScriptBehaviorOverrides(
    const TinyScriptDefinition& script,
    const Peter::AI::CompanionWorldContext& context);

  [[nodiscard]] CreatorReplaySnippet BuildCreatorReplaySnippet(
    std::string_view scenarioId,
    const std::vector<std::string>& beforeTimeline,
    const std::vector<std::string>& afterTimeline,
    std::string_view changeSummary);
  [[nodiscard]] CreatorActivationResult ActivateCreatorArtifact(
    CreatorManifest& manifest,
    std::string_view kindId,
    std::string_view contentId,
    int revision,
    bool validationPassed);
  void DisableCreatorArtifact(
    CreatorManifest& manifest,
    std::string_view kindId,
    std::string_view contentId);

  [[nodiscard]] Peter::Core::StructuredFields ToSaveFields(const CreatorManifest& manifest);
  [[nodiscard]] CreatorManifest CreatorManifestFromSaveFields(const Peter::Core::StructuredFields& fields);
  [[nodiscard]] Peter::Core::StructuredFields ToSaveFields(const CreatorProgressState& progressState);
  [[nodiscard]] CreatorProgressState CreatorProgressStateFromSaveFields(
    const Peter::Core::StructuredFields& fields);
  [[nodiscard]] Peter::Core::StructuredFields ToSaveFields(const CreatorSettings& settings);
  [[nodiscard]] CreatorSettings CreatorSettingsFromSaveFields(const Peter::Core::StructuredFields& fields);

  [[nodiscard]] std::string_view ToString(TinyScriptHookKind hookKind);
} // namespace Peter::Workshop
