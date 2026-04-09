#pragma once

#include "Phase1Slice.h"

#include "PeterCore/EventBus.h"
#include "PeterCore/ProfileService.h"
#include "PeterCore/QualityProfile.h"
#include "PeterCore/SaveDomainStore.h"
#include "PeterTraversal/TraversalProfile.h"
#include "PeterWorld/WorldQueryService.h"

namespace Peter::App
{
  class PlayableSessionController
  {
  public:
    PlayableSessionController(
      Peter::Adapters::PlatformServices& platform,
      Peter::Core::EventBus& eventBus,
      Peter::Core::ProfileInfo profile,
      Peter::Core::SaveDomainStore& saveDomainStore);

    [[nodiscard]] SliceRunReport Run(std::string_view scenario);

  private:
    struct PersistentState
    {
      Peter::AI::CompanionConfig companionConfig;
      Peter::UI::AccessibilitySettings accessibility;
      int completedRaids = 0;
      std::string lastRaidResult = "none";
      std::string lastMissionId = "mission.salvage_run.machine_silo";
      std::string lastSceneId = "scene.playable.home_base";
    };

    struct SegmentResult
    {
      Peter::Traversal::TraversalState traversalState;
      Peter::World::WorldFrameSnapshot snapshot;
      Peter::World::InteractionCandidate interaction;
      double inputToMotionLatencyMs = 0.0;
      double interactionHitchMs = 0.0;
    };

    [[nodiscard]] PersistentState LoadPersistentState() const;
    void SaveCompanionAndAccessibility(const PersistentState& state) const;
    void SaveMissionBoundary(const PersistentState& state) const;
    void SaveProfileBoundary(const PersistentState& state) const;
    [[nodiscard]] SegmentResult TraverseToInteraction(
      Peter::World::IWorldQueryService& worldQueryService,
      Peter::Traversal::TraversalState startState,
      std::string_view sceneId,
      std::string_view interactionId,
      std::string_view objectiveId) const;
    [[nodiscard]] Peter::World::ExtractionRuntimeState RunExtractionCountdown(
      const PersistentState& persistentState,
      std::string_view roomId,
      std::string_view interactionId,
      bool allowSuccess) const;
    [[nodiscard]] Peter::AI::CompanionDecisionSnapshot BuildPlayableCompanionDecision(
      const PersistentState& persistentState,
      std::string_view currentGoal,
      std::string_view roomId,
      double distanceToPlayerMeters,
      double distanceToThreatMeters,
      bool extractionActive) const;

    Peter::Adapters::PlatformServices& m_platform;
    Peter::Core::EventBus& m_eventBus;
    Peter::Core::ProfileInfo m_profile;
    Peter::Core::SaveDomainStore& m_saveDomainStore;
    Peter::Core::Phase7PlayableQualityProfile m_qualityProfile;
    Peter::Traversal::TraversalProfile m_traversalProfile;
  };
} // namespace Peter::App
