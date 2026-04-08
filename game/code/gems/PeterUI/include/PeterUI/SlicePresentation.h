#pragma once

#include "PeterAI/CompanionAi.h"
#include "PeterAdapters/PlatformServices.h"
#include "PeterCore/EventBus.h"
#include "PeterProgression/Crafting.h"
#include "PeterWorld/SliceContent.h"

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
    double cameraSensitivity = 1.0;
    bool invertY = false;
    bool holdToInteract = true;
    bool reducedTimePressure = false;
    bool lightAimAssist = true;
  };

  [[nodiscard]] std::string RenderHomeBaseOverview(const Peter::World::HomeBaseDefinition& homeBase);
  [[nodiscard]] std::string RenderMissionBoard(
    const std::vector<Peter::World::MissionTemplateDefinition>& missions,
    std::string_view selectedMissionId);
  [[nodiscard]] std::string RenderRaidZoneOverview(
    const Peter::World::RaidZoneDefinition& raidZone,
    const Peter::World::MissionTemplateDefinition& mission);
  [[nodiscard]] std::string RenderCompanionExplainPanel(const Peter::AI::CompanionDecisionSnapshot& snapshot);
  [[nodiscard]] std::string RenderPostRaidSummary(const Peter::World::RaidSummary& summary);
  [[nodiscard]] std::string RenderTutorialLesson(
    const Peter::World::TutorialLessonDefinition& lesson,
    int hintLevel);
  [[nodiscard]] std::string RenderAccessibilitySettings(
    const AccessibilitySettings& settings,
    const std::vector<Peter::Adapters::ActionBinding>& bindings);
  [[nodiscard]] std::string RenderWorkshopTracks(
    const std::vector<Peter::Progression::UpgradeTrackDefinition>& tracks,
    const Peter::Progression::WorkshopState& workshopState);
  [[nodiscard]] Peter::Core::StructuredFields ToSaveFields(const AccessibilitySettings& settings);
  [[nodiscard]] AccessibilitySettings AccessibilitySettingsFromSaveFields(
    const Peter::Core::StructuredFields& fields,
    const std::vector<Peter::Adapters::ActionBinding>& defaults);
} // namespace Peter::UI
