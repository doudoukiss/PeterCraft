#pragma once

#include "PeterAI/CompanionAi.h"
#include "PeterAdapters/PlatformServices.h"
#include "PeterCore/CreatorContentStore.h"
#include "PeterCore/EventBus.h"
#include "PeterCore/ProfileService.h"
#include "PeterCore/QualityProfile.h"
#include "PeterCore/SaveDomainStore.h"
#include "PeterCore/SliceTypes.h"
#include "PeterInventory/InventoryState.h"
#include "PeterProgression/Crafting.h"
#include "PeterTraversal/TraversalProfile.h"
#include "PeterUI/SlicePresentation.h"
#include "PeterWorkshop/CreatorWorkshop.h"
#include "PeterWorkshop/WorkshopTuning.h"
#include "PeterWorld/SliceContent.h"

#include <string>
#include <string_view>
#include <vector>

namespace Peter::App
{
  enum class SliceScenario
  {
    GuidedFirstRun,
    HappyPath,
    FailurePath,
    ArtifactRecovery,
    EscortSupport,
    Smoke
  };

  [[nodiscard]] SliceScenario ParseScenario(std::string_view scenarioName);
  [[nodiscard]] std::string_view ToString(SliceScenario scenario);

  struct SliceRunReport
  {
    bool success = false;
    std::string summary;
    std::string missionId;
    Peter::AI::CompanionDecisionSnapshot lastCompanionDecision;
    Peter::Workshop::RuleEditPreview lastRuleEditPreview;
    Peter::Core::ExtractionResult extractionResult;
    Peter::World::RaidSummary raidSummary;
    std::string currentSceneId;
    std::string roomId;
    std::string activeInteractionId;
    std::string extractionState = "idle";
    std::string inputScheme = "mouse_keyboard";
    std::string cameraMode = "third_person_ots";
    double playerPositionX = 0.0;
    double playerPositionY = 0.0;
    double playerPositionZ = 0.0;
    double playerVelocityX = 0.0;
    double playerVelocityY = 0.0;
    double playerVelocityZ = 0.0;
    double playerSpeedMetersPerSecond = 0.0;
    double inputToMotionLatencyMs = 0.0;
    double interactionHitchMs = 0.0;
    double transitionMs = 0.0;
  };

  class Phase1Slice
  {
  public:
    Phase1Slice(
      Peter::Adapters::PlatformServices& platform,
      Peter::Core::EventBus& eventBus,
      Peter::Core::ProfileInfo profile,
      Peter::Core::SaveDomainStore& saveDomainStore);

    [[nodiscard]] SliceRunReport Run(SliceScenario scenario);

  private:
    struct PersistentState
    {
      Peter::Inventory::InventoryState inventory;
      Peter::Inventory::LoadoutState loadout;
      Peter::Inventory::RecoveryState recovery;
      Peter::Progression::WorkshopState workshop;
      Peter::AI::CompanionConfig companionConfig;
      Peter::UI::AccessibilitySettings accessibility;
      Peter::Workshop::CreatorManifest creatorManifest;
      Peter::Workshop::CreatorProgressState creatorProgress;
      Peter::Workshop::CreatorSettings creatorSettings;
      std::map<std::string, double, std::less<>> tinkerValues = Peter::Workshop::DefaultTinkerValues();
      std::vector<std::string> completedLessons;
      bool guidedFirstRunComplete = false;
      bool ruleEditComplete = false;
      int tutorialHintLevel = 0;
      int completedRaids = 0;
      std::string lastRaidResult = "none";
      std::string lastMissionId = "mission.salvage_run.machine_silo";
      std::string lastSceneId = "scene.raid.machine_silo";
    };

    [[nodiscard]] PersistentState LoadPersistentState() const;
    void SavePersistentState(const PersistentState& state) const;
    void PresentHomeBase(const PersistentState& state) const;
    [[nodiscard]] const Peter::World::StationDefinition& FindStation(std::string_view stationId) const;
    void VisitStation(const Peter::World::StationDefinition& station) const;
    void EmitCompanionDecision(
      std::string_view roomId,
      const Peter::AI::CompanionDecisionSnapshot& snapshot) const;
    void RunLesson(PersistentState& state, std::string_view lessonId, bool replayed) const;
    [[nodiscard]] SliceRunReport RunMissionScenario(
      PersistentState& state,
      std::string_view missionId,
      bool expectSuccess,
      bool guidedMode);

    Peter::Adapters::PlatformServices& m_platform;
    Peter::Core::EventBus& m_eventBus;
    Peter::Core::ProfileInfo m_profile;
    Peter::Core::SaveDomainStore& m_saveDomainStore;
    Peter::Core::CreatorContentStore m_creatorContentStore;
    Peter::Core::Phase6QualityProfile m_qualityProfile;
    Peter::World::HomeBaseDefinition m_homeBase;
    Peter::World::RaidZoneDefinition m_raidZone;
    Peter::Traversal::TraversalProfile m_traversal;
  };
} // namespace Peter::App
