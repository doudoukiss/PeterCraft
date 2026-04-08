#pragma once

#include "PeterAI/CompanionAi.h"
#include "PeterAdapters/PlatformServices.h"
#include "PeterCore/EventBus.h"
#include "PeterCore/ProfileService.h"
#include "PeterCore/SaveDomainStore.h"
#include "PeterCore/SliceTypes.h"
#include "PeterInventory/InventoryState.h"
#include "PeterProgression/Crafting.h"
#include "PeterTraversal/TraversalProfile.h"
#include "PeterWorkshop/WorkshopTuning.h"
#include "PeterWorld/SliceContent.h"

#include <string>
#include <string_view>

namespace Peter::App
{
  enum class SliceScenario
  {
    GuidedFirstRun,
    HappyPath,
    FailurePath,
    Smoke
  };

  [[nodiscard]] SliceScenario ParseScenario(std::string_view scenarioName);
  [[nodiscard]] std::string_view ToString(SliceScenario scenario);

  struct SliceRunReport
  {
    bool success = false;
    std::string summary;
    Peter::AI::CompanionDecisionSnapshot lastCompanionDecision;
    Peter::Workshop::RuleEditPreview lastRuleEditPreview;
    Peter::Core::ExtractionResult extractionResult;
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
      Peter::Progression::WorkshopState workshop;
      Peter::AI::CompanionConfig companionConfig;
      bool guidedFirstRunComplete = false;
      bool ruleEditComplete = false;
      int completedRaids = 0;
      std::string lastRaidResult = "none";
    };

    [[nodiscard]] PersistentState LoadPersistentState() const;
    void SavePersistentState(const PersistentState& state) const;
    void PresentHomeBase() const;
    [[nodiscard]] const Peter::World::StationDefinition& FindStation(std::string_view stationId) const;
    void VisitStation(const Peter::World::StationDefinition& station) const;
    void EmitCompanionDecision(
      std::string_view roomId,
      const Peter::AI::CompanionDecisionSnapshot& snapshot) const;
    [[nodiscard]] SliceRunReport RunSuccessSlice(PersistentState& state, bool guidedMode);
    [[nodiscard]] SliceRunReport RunFailureSlice(PersistentState& state);

    Peter::Adapters::PlatformServices& m_platform;
    Peter::Core::EventBus& m_eventBus;
    Peter::Core::ProfileInfo m_profile;
    Peter::Core::SaveDomainStore& m_saveDomainStore;
    Peter::World::HomeBaseDefinition m_homeBase;
    Peter::World::RaidZoneDefinition m_raidZone;
    Peter::Traversal::TraversalProfile m_traversal;
  };
} // namespace Peter::App
