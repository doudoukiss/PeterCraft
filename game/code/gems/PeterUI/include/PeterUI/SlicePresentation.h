#pragma once

#include "PeterAI/CompanionAi.h"
#include "PeterAdapters/PlatformServices.h"
#include "PeterCore/EventBus.h"
#include "PeterProgression/Crafting.h"
#include "PeterTraversal/TraversalProfile.h"
#include "PeterWorkshop/CreatorWorkshop.h"
#include "PeterWorld/SliceContent.h"
#include "PeterWorld/WorldQueryService.h"

#include <map>
#include <string>
#include <vector>

namespace Peter::UI
{
  struct AccessibilitySettings
  {
    std::map<std::string, std::string, std::less<>> actionBindings;
    int textScalePercent = 100;
    bool subtitlesEnabled = true;
    int subtitleScalePercent = 100;
    bool subtitleBackgroundEnabled = true;
    double cameraSensitivity = 1.0;
    bool invertY = false;
    bool holdToInteract = true;
    bool reducedTimePressure = false;
    bool lightAimAssist = true;
    bool highContrastEnabled = false;
    bool iconRedundancyEnabled = true;
    bool motionComfortEnabled = false;
  };

  struct OnboardingCheckpoint
  {
    std::string id;
    std::string labelTextId;
    std::string helpTextId;
    bool completed = false;
  };

  [[nodiscard]] std::string RenderHomeBaseOverview(const Peter::World::HomeBaseDefinition& homeBase);
  [[nodiscard]] std::string RenderMissionBoard(
    const std::vector<Peter::World::MissionTemplateDefinition>& missions,
    std::string_view selectedMissionId);
  [[nodiscard]] std::string RenderMissionBlueprintPreview(
    const Peter::World::MissionBlueprintDefinition& blueprint,
    const Peter::World::RaidZoneDefinition& raidZone);
  [[nodiscard]] std::string RenderEncounterPatternPreview(
    const Peter::World::EncounterPatternDefinition& encounter);
  [[nodiscard]] std::string RenderRaidZoneOverview(
    const Peter::World::RaidZoneDefinition& raidZone,
    const Peter::World::MissionTemplateDefinition& mission);
  [[nodiscard]] std::string RenderCompanionBehaviorEditor(const Peter::AI::CompanionConfig& config);
  [[nodiscard]] std::string RenderCompanionExplainPanel(const Peter::AI::CompanionDecisionSnapshot& snapshot);
  [[nodiscard]] std::string RenderCompanionCompareView(
    const Peter::AI::CompanionDecisionSnapshot& before,
    const Peter::AI::CompanionDecisionSnapshot& after,
    std::string_view previewSummary);
  [[nodiscard]] std::string RenderAiDebugPanel(const Peter::AI::AgentExplainSnapshot& snapshot);
  [[nodiscard]] std::string RenderPostRaidSummary(const Peter::World::RaidSummary& summary);
  [[nodiscard]] std::string RenderTutorialLesson(
    const Peter::World::TutorialLessonDefinition& lesson,
    int hintLevel);
  [[nodiscard]] std::string ResolveChildFacingText(std::string_view textId);
  [[nodiscard]] std::string ResolveActionDisplayLabel(const Peter::Adapters::ActionBinding& binding);
  [[nodiscard]] std::string RenderAccessibilitySettings(
    const AccessibilitySettings& settings,
    const std::vector<Peter::Adapters::ActionBinding>& bindings);
  [[nodiscard]] std::string RenderOnboardingFunnel(const std::vector<OnboardingCheckpoint>& checkpoints);
  [[nodiscard]] std::string RenderWorkshopTracks(
    const std::vector<Peter::Progression::UpgradeTrackDefinition>& tracks,
    const Peter::Progression::WorkshopState& workshopState);
  [[nodiscard]] std::string RenderCreatorPanel(
    const std::map<std::string, double, std::less<>>& tinkerValues,
    const Peter::Workshop::LogicRulesetDefinition* activeRuleset,
    const Peter::Workshop::TinyScriptDefinition* activeScript);
  [[nodiscard]] std::string RenderTraversalDebugPanel(const Peter::Traversal::TraversalState& state);
  [[nodiscard]] std::string RenderInteractionPrompt(
    const Peter::World::InteractionCandidate& candidate,
    std::string_view inputScheme);
  [[nodiscard]] std::string RenderExtractionStatus(const Peter::World::ExtractionRuntimeState& state);
  [[nodiscard]] std::string RenderTinyScriptEditor(const Peter::Workshop::TinyScriptDefinition& script);
  [[nodiscard]] std::string RenderReplaySnippet(const Peter::Workshop::CreatorReplaySnippet& snippet);
  [[nodiscard]] std::string RenderMentorSummary(
    const Peter::Workshop::CreatorManifest& manifest,
    const Peter::Workshop::CreatorProgressState& progress,
    const Peter::AI::AgentExplainSnapshot& snapshot);
  [[nodiscard]] std::string RenderCreatorValidationMessage(bool valid, std::string_view summary);
  [[nodiscard]] Peter::Core::StructuredFields ToSaveFields(const AccessibilitySettings& settings);
  [[nodiscard]] AccessibilitySettings AccessibilitySettingsFromSaveFields(
    const Peter::Core::StructuredFields& fields,
    const std::vector<Peter::Adapters::ActionBinding>& defaults);
} // namespace Peter::UI
