#pragma once

#include "PeterAI/CompanionAi.h"
#include "PeterInventory/InventoryState.h"

#include <string>
#include <string_view>
#include <vector>

namespace Peter::World
{
  struct StationDefinition
  {
    std::string id;
    std::string displayName;
    std::string helpText;
    std::string panelId;
  };

  struct HomeBaseDefinition
  {
    std::string sceneId;
    std::string displayName;
    std::vector<StationDefinition> stations;
  };

  struct RoomDefinition
  {
    std::string id;
    std::string displayName;
    std::string roomType;
    std::string landmarkLabel;
    std::vector<std::string> exits;
    bool optionalPath = false;
    bool highRiskReward = false;
    bool extractionPoint = false;
  };

  struct EncounterDefinition
  {
    std::string id;
    std::string roomId;
    std::vector<Peter::AI::EnemyUnit> enemies;
    bool optional = false;
  };

  struct ExtractionSettings
  {
    std::string id;
    int countdownSeconds = 5;
    std::string successCueId;
    std::string failCueId;
  };

  struct MissionObjectiveDefinition
  {
    std::string id;
    std::string kind;
    std::string targetId;
    std::string description;
    int requiredCount = 1;
    bool optional = false;
  };

  struct RewardBundleDefinition
  {
    Peter::Inventory::ItemLedger guaranteedItems;
    std::string upgradeTrackId;
    std::string lessonTipId;
  };

  struct MissionTemplateDefinition
  {
    std::string id;
    std::string displayName;
    std::string templateType;
    std::string zoneId;
    std::vector<std::string> roomIds;
    std::vector<MissionObjectiveDefinition> objectives;
    std::vector<MissionObjectiveDefinition> sideObjectives;
    RewardBundleDefinition rewardBundle;
    std::string failRuleId;
    int recommendedMinutes = 10;
    int extractionCountdownSeconds = 5;
  };

  struct TutorialStepDefinition
  {
    std::string id;
    std::string action;
    std::string prompt;
    std::string triggerEvent;
    std::string completionEvent;
    std::string targetId;
    int hintThreshold = 0;
  };

  struct TutorialLessonDefinition
  {
    std::string id;
    std::string displayName;
    std::string summary;
    bool replayable = true;
    std::vector<TutorialStepDefinition> steps;
  };

  struct RaidSummary
  {
    bool success = false;
    std::string missionId;
    std::string missionDisplayName;
    std::vector<std::string> timeline;
    std::string gainedItems;
    std::string lostItems;
    std::string brokenItems;
    std::string recoveredItems;
    std::string companionHighlight;
    std::string lessonTip;
  };

  struct RaidZoneDefinition
  {
    std::string sceneId;
    std::string missionId;
    std::string displayName;
    std::string entryRoomId;
    std::string extractionRoomId;
    std::vector<RoomDefinition> rooms;
    std::vector<EncounterDefinition> encounters;
    ExtractionSettings extraction;
  };

  [[nodiscard]] HomeBaseDefinition BuildPhase1HomeBase();
  [[nodiscard]] RaidZoneDefinition BuildPhase1RaidZone();
  [[nodiscard]] const std::vector<MissionTemplateDefinition>& BuildPhase2MissionTemplates();
  [[nodiscard]] const MissionTemplateDefinition* FindMissionTemplate(std::string_view missionId);
  [[nodiscard]] const std::vector<TutorialLessonDefinition>& BuildPhase2TutorialLessons();
  [[nodiscard]] const TutorialLessonDefinition* FindTutorialLesson(std::string_view lessonId);
} // namespace Peter::World
