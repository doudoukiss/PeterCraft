#pragma once

#include "PeterAI/CompanionAi.h"
#include "PeterInventory/InventoryState.h"

#include <filesystem>
#include <string>
#include <string_view>
#include <vector>

namespace Peter::Workshop
{
  struct MiniMissionDraftDefinition;
}

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

  struct RoomKitDefinition
  {
    std::string id;
    std::string displayName;
    std::string archetype;
    std::string connectorClass;
    int widthMeters = 8;
    int depthMeters = 8;
    int heightMeters = 5;
    std::string navExpectation;
    std::string lightingExpectation;
    std::string reviewRecordId;
  };

  struct RoomVariantDefinition
  {
    std::string id;
    std::string roomId;
    std::string displayName;
    std::string kitId;
    std::string styleProfileId;
    std::string roomType;
    std::string landmarkLabel;
    std::vector<std::string> exitRoomIds;
    std::vector<std::string> tutorialAnchorIds;
    std::vector<std::string> companionHintAnchorIds;
    std::vector<std::string> feedbackTagIds;
    bool optionalPath = false;
    bool highRiskReward = false;
    bool extractionPoint = false;
    std::string reviewRecordId;
  };

  struct EncounterDefinition
  {
    std::string id;
    std::string roomId;
    std::vector<Peter::AI::EnemyUnit> enemies;
    bool optional = false;
  };

  struct EncounterPatternDefinition
  {
    std::string id;
    std::string displayName;
    std::string roomId;
    std::vector<Peter::AI::EnemyUnit> enemies;
    std::vector<std::string> lootItemIds;
    std::vector<std::string> hazardIds;
    std::vector<std::string> landmarkIds;
    std::vector<std::string> companionHintMomentIds;
    std::vector<std::string> tutorialHookIds;
    std::vector<std::string> feedbackTagIds;
    std::string pressureBeatId;
    bool optional = false;
    std::string reviewRecordId;
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

  struct FeedbackTagDefinition
  {
    std::string id;
    std::string displayName;
    std::string category;
    std::string cueFamily;
    std::string cueVariant;
    std::string summary;
  };

  struct WorldStyleProfileDefinition
  {
    std::string id;
    std::string displayName;
    std::string visualMotif;
    std::string propLanguage;
    std::string colorHierarchy;
    std::string signageGrammar;
    std::string factionStyle;
  };

  struct MissionBlueprintDefinition
  {
    std::string id;
    std::string displayName;
    std::string templateFamilyId;
    std::string zoneId;
    std::string sceneId;
    std::string zoneDisplayName;
    std::vector<std::string> roomVariantIds;
    std::vector<std::string> encounterPatternIds;
    std::vector<MissionObjectiveDefinition> objectives;
    std::vector<MissionObjectiveDefinition> sideObjectives;
    RewardBundleDefinition rewardBundle;
    std::string failRuleId;
    int recommendedMinutes = 10;
    int extractionCountdownSeconds = 5;
    std::vector<std::string> feedbackTagIds;
    std::vector<std::string> tutorialHookIds;
    std::string reviewRecordId;
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

  struct ShippableContentManifest
  {
    std::string id;
    std::vector<std::string> roomVariantIds;
    std::vector<std::string> encounterPatternIds;
    std::vector<std::string> missionBlueprintIds;
    std::vector<std::string> styleProfileIds;
  };

  struct RoomMetricsSummary
  {
    std::string roomVariantId;
    std::string connectorClass;
    int widthMeters = 0;
    int depthMeters = 0;
    int heightMeters = 0;
    std::size_t exitCount = 0;
    bool extractionPoint = false;
    bool highRiskReward = false;
  };

  struct MiniMissionRoomBundleDefinition
  {
    std::string id;
    std::string displayName;
    std::string entryRoomId;
    std::string extractionRoomId;
    std::vector<std::string> roomIds;
  };

  struct MiniMissionEnemyGroupDefinition
  {
    std::string id;
    std::string displayName;
    std::vector<Peter::AI::EnemyUnit> enemies;
  };

  struct MiniMissionRewardDefinition
  {
    std::string id;
    std::string displayName;
    RewardBundleDefinition rewardBundle;
  };

  [[nodiscard]] HomeBaseDefinition BuildPhase1HomeBase();
  [[nodiscard]] RaidZoneDefinition BuildPhase1RaidZone();
  [[nodiscard]] RaidZoneDefinition BuildRaidZoneForMission(std::string_view missionId);
  [[nodiscard]] std::filesystem::path ResolveContentRoot();
  [[nodiscard]] const std::vector<RoomKitDefinition>& BuildPhase5RoomKits();
  [[nodiscard]] const RoomKitDefinition* FindRoomKit(std::string_view roomKitId);
  [[nodiscard]] const std::vector<RoomVariantDefinition>& BuildPhase5RoomVariants();
  [[nodiscard]] const RoomVariantDefinition* FindRoomVariant(std::string_view roomVariantId);
  [[nodiscard]] const std::vector<EncounterPatternDefinition>& BuildPhase5EncounterPatterns();
  [[nodiscard]] const EncounterPatternDefinition* FindEncounterPattern(std::string_view encounterPatternId);
  [[nodiscard]] const std::vector<FeedbackTagDefinition>& BuildPhase5FeedbackTags();
  [[nodiscard]] const FeedbackTagDefinition* FindFeedbackTag(std::string_view feedbackTagId);
  [[nodiscard]] const std::vector<WorldStyleProfileDefinition>& BuildPhase5StyleProfiles();
  [[nodiscard]] const WorldStyleProfileDefinition* FindWorldStyleProfile(std::string_view styleProfileId);
  [[nodiscard]] const std::vector<MissionBlueprintDefinition>& BuildPhase5MissionBlueprints();
  [[nodiscard]] const MissionBlueprintDefinition* FindMissionBlueprint(std::string_view missionBlueprintId);
  [[nodiscard]] const ShippableContentManifest& BuildPhase5ShippableContentManifest();
  [[nodiscard]] RoomMetricsSummary BuildRoomMetricsSummary(std::string_view roomVariantId);
  [[nodiscard]] const std::vector<MissionTemplateDefinition>& BuildPhase2MissionTemplates();
  [[nodiscard]] const MissionTemplateDefinition* FindMissionTemplate(std::string_view missionId);
  [[nodiscard]] const std::vector<TutorialLessonDefinition>& BuildPhase2TutorialLessons();
  [[nodiscard]] const TutorialLessonDefinition* FindTutorialLesson(std::string_view lessonId);
  [[nodiscard]] const std::vector<MiniMissionRoomBundleDefinition>& BuildPhase4MiniMissionRoomBundles();
  [[nodiscard]] const MiniMissionRoomBundleDefinition* FindMiniMissionRoomBundle(std::string_view bundleId);
  [[nodiscard]] const std::vector<MiniMissionEnemyGroupDefinition>& BuildPhase4MiniMissionEnemyGroups();
  [[nodiscard]] const MiniMissionEnemyGroupDefinition* FindMiniMissionEnemyGroup(std::string_view enemyGroupId);
  [[nodiscard]] const std::vector<MiniMissionRewardDefinition>& BuildPhase4MiniMissionRewards();
  [[nodiscard]] const MiniMissionRewardDefinition* FindMiniMissionReward(std::string_view rewardId);
  [[nodiscard]] MissionTemplateDefinition BuildMissionFromMiniMissionDraft(
    const Peter::Workshop::MiniMissionDraftDefinition& draft);
} // namespace Peter::World
