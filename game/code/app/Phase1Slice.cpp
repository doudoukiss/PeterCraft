#include "Phase1Slice.h"

#include "PeterCombat/EncounterSimulator.h"
#include "PeterUI/SlicePresentation.h"
#include "PeterValidation/ValidationModule.h"
#include "PeterWorld/SceneShell.h"

#include <sstream>
#include <stdexcept>

namespace Peter::App
{
  namespace
  {
    constexpr std::string_view kProfileMetaDomain = "save_domain.profile_meta";
    constexpr std::string_view kInventoryDomain = "save_domain.inventory";
    constexpr std::string_view kWorkshopDomain = "save_domain.workshop_upgrades";
    constexpr std::string_view kMissionDomain = "save_domain.mission_progress";
    constexpr std::string_view kTutorialDomain = "save_domain.tutorial_progress";
    constexpr std::string_view kCompanionDomain = "save_domain.companion_config";

    Peter::Core::SliceItemLedger ToSliceLedger(const Peter::Inventory::ItemLedger& ledger)
    {
      Peter::Core::SliceItemLedger copy;
      for (const auto& [itemId, quantity] : ledger)
      {
        copy[itemId] = quantity;
      }
      return copy;
    }

    std::string SummarizeSliceLedger(const Peter::Core::SliceItemLedger& ledger)
    {
      Peter::Inventory::ItemLedger copy;
      for (const auto& [itemId, quantity] : ledger)
      {
        copy[itemId] = quantity;
      }
      return Peter::Inventory::SummarizeLedger(copy);
    }
  } // namespace

  SliceScenario ParseScenario(const std::string_view scenarioName)
  {
    if (scenarioName == "happy_path")
    {
      return SliceScenario::HappyPath;
    }

    if (scenarioName == "failure_path")
    {
      return SliceScenario::FailurePath;
    }

    if (scenarioName == "smoke")
    {
      return SliceScenario::Smoke;
    }

    return SliceScenario::GuidedFirstRun;
  }

  std::string_view ToString(const SliceScenario scenario)
  {
    switch (scenario)
    {
      case SliceScenario::GuidedFirstRun:
        return "guided_first_run";
      case SliceScenario::HappyPath:
        return "happy_path";
      case SliceScenario::FailurePath:
        return "failure_path";
      case SliceScenario::Smoke:
        return "smoke";
    }

    return "guided_first_run";
  }

  Phase1Slice::Phase1Slice(
    Peter::Adapters::PlatformServices& platform,
    Peter::Core::EventBus& eventBus,
    Peter::Core::ProfileInfo profile,
    Peter::Core::SaveDomainStore& saveDomainStore)
    : m_platform(platform)
    , m_eventBus(eventBus)
    , m_profile(std::move(profile))
    , m_saveDomainStore(saveDomainStore)
    , m_homeBase(Peter::World::BuildPhase1HomeBase())
    , m_raidZone(Peter::World::BuildPhase1RaidZone())
    , m_traversal(Peter::Traversal::BuildPhase1TraversalProfile())
  {
  }

  Phase1Slice::PersistentState Phase1Slice::LoadPersistentState() const
  {
    PersistentState state;

    if (m_saveDomainStore.DomainExists(kInventoryDomain))
    {
      Peter::Inventory::LoadFromSaveFields(
        m_saveDomainStore.ReadDomain(std::string(kInventoryDomain)),
        state.inventory,
        state.loadout);
    }
    else
    {
      state.inventory.carrySlotCapacity = Peter::Inventory::MaxCarrySlots(state.loadout);
    }

    if (m_saveDomainStore.DomainExists(kWorkshopDomain))
    {
      state.workshop = Peter::Progression::WorkshopStateFromSaveFields(
        m_saveDomainStore.ReadDomain(std::string(kWorkshopDomain)));
    }

    if (m_saveDomainStore.DomainExists(kCompanionDomain))
    {
      state.companionConfig = Peter::AI::CompanionConfigFromSaveFields(
        m_saveDomainStore.ReadDomain(std::string(kCompanionDomain)));
    }

    if (m_saveDomainStore.DomainExists(kProfileMetaDomain))
    {
      const auto fields = m_saveDomainStore.ReadDomain(std::string(kProfileMetaDomain));
      const auto completedRaids = fields.find("completed_raids");
      const auto lastResult = fields.find("last_raid_result");
      state.completedRaids = completedRaids == fields.end() ? 0 : std::stoi(completedRaids->second);
      state.lastRaidResult = lastResult == fields.end() ? "none" : lastResult->second;
    }

    if (m_saveDomainStore.DomainExists(kTutorialDomain))
    {
      const auto fields = m_saveDomainStore.ReadDomain(std::string(kTutorialDomain));
      const auto guided = fields.find("guided_first_run_complete");
      const auto ruleEdit = fields.find("rule_edit_complete");
      state.guidedFirstRunComplete = guided != fields.end() && guided->second == "true";
      state.ruleEditComplete = ruleEdit != fields.end() && ruleEdit->second == "true";
    }

    if (state.workshop.salvagePouchCrafted && state.loadout.carrySlotBonus == 0)
    {
      state.loadout.carrySlotBonus = 1;
      state.loadout.salvagePouchEquipped = true;
      state.inventory.carrySlotCapacity = Peter::Inventory::MaxCarrySlots(state.loadout);
    }

    return state;
  }

  void Phase1Slice::SavePersistentState(const PersistentState& state) const
  {
    m_saveDomainStore.WriteDomain(
      std::string(kInventoryDomain),
      Peter::Inventory::ToSaveFields(state.inventory, state.loadout));
    m_saveDomainStore.WriteDomain(
      std::string(kWorkshopDomain),
      Peter::Progression::ToSaveFields(state.workshop));
    m_saveDomainStore.WriteDomain(
      std::string(kCompanionDomain),
      Peter::AI::ToSaveFields(state.companionConfig));
    m_saveDomainStore.WriteDomain(
      std::string(kProfileMetaDomain),
      Peter::Core::StructuredFields{
        {"completed_raids", std::to_string(state.completedRaids)},
        {"last_raid_result", state.lastRaidResult},
        {"profile_id", m_profile.profileId}
      });
    m_saveDomainStore.WriteDomain(
      std::string(kTutorialDomain),
      Peter::Core::StructuredFields{
        {"guided_first_run_complete", state.guidedFirstRunComplete ? "true" : "false"},
        {"rule_edit_complete", state.ruleEditComplete ? "true" : "false"}
      });
    m_saveDomainStore.WriteDomain(
      std::string(kMissionDomain),
      Peter::Core::StructuredFields{
        {"last_mission_id", m_raidZone.missionId},
        {"last_scene_id", m_raidZone.sceneId},
        {"last_result", state.lastRaidResult}
      });
  }

  void Phase1Slice::PresentHomeBase() const
  {
    m_platform.camera->ApplyRig(m_traversal.cameraRig);
    m_platform.ui->PresentState("ui.home_base");
    m_platform.ui->PresentPanel("home_base.overview", Peter::UI::RenderHomeBaseOverview(m_homeBase));
    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::Gameplay,
      "gameplay.traversal.ready",
      {
        {"camera_mode", m_traversal.cameraRig.mode},
        {"interaction_range_meters", std::to_string(m_traversal.interactionRangeMeters)},
        {"input_scheme", m_platform.input->ActiveScheme()}
      }});
  }

  const Peter::World::StationDefinition& Phase1Slice::FindStation(const std::string_view stationId) const
  {
    for (const auto& station : m_homeBase.stations)
    {
      if (station.id == stationId)
      {
        return station;
      }
    }

    throw std::runtime_error("Missing station definition.");
  }

  void Phase1Slice::VisitStation(const Peter::World::StationDefinition& station) const
  {
    m_platform.ui->PresentPrompt(station.helpText);
    m_platform.ui->PresentPanel(station.panelId, station.displayName + ": " + station.helpText);
    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::CreatorTools,
      "creator_tools.station.opened",
      {
        {"station_id", station.id},
        {"station_name", station.displayName}
      }});
  }

  void Phase1Slice::EmitCompanionDecision(
    const std::string_view roomId,
    const Peter::AI::CompanionDecisionSnapshot& snapshot) const
  {
    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::AI,
      "ai.companion.action",
      {
        {"room_id", std::string(roomId)},
        {"state", snapshot.currentState},
        {"action", snapshot.lastAction},
        {"reason", snapshot.topReason}
      }});
  }

  SliceRunReport Phase1Slice::Run(SliceScenario scenario)
  {
    PresentHomeBase();
    auto state = LoadPersistentState();

    switch (scenario)
    {
      case SliceScenario::GuidedFirstRun:
        return RunSuccessSlice(state, true);
      case SliceScenario::HappyPath:
      case SliceScenario::Smoke:
        return RunSuccessSlice(state, false);
      case SliceScenario::FailurePath:
        return RunFailureSlice(state);
    }

    return RunSuccessSlice(state, false);
  }

  SliceRunReport Phase1Slice::RunSuccessSlice(PersistentState& state, const bool guidedMode)
  {
    Peter::World::SceneShell sceneShell(m_eventBus);
    const auto homeScene = sceneShell.LoadHomeBase(m_homeBase);
    (void)homeScene;

    const auto& missionBoard = FindStation("station.home.mission_board");
    VisitStation(missionBoard);

    if (guidedMode)
    {
      m_platform.ui->PresentPanel(
        "tutorial.first_run",
        "Launch the guided first mission, collect salvage, extract safely, then tune your companion.");
      m_eventBus.Emit(Peter::Core::Event{
        Peter::Core::EventCategory::Gameplay,
        "gameplay.tutorial.started",
        {{"tutorial_id", "tutorial.vertical_slice.guided_first_run"}}});
    }

    const auto raidScene = sceneShell.LoadRaidZone(m_raidZone);
    (void)raidScene;
    m_platform.ui->PresentState("ui.raid_hud");
    m_platform.ui->PresentPanel("raid.overview", Peter::UI::RenderRaidZoneOverview(m_raidZone));

    Peter::Core::RaidSessionState raid;
    raid.missionId = m_raidZone.missionId;
    raid.sceneId = m_raidZone.sceneId;
    raid.currentRoomId = m_raidZone.entryRoomId;
    raid.playerHealth = 100;

    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::Gameplay,
      "gameplay.raid.start",
      {
        {"guided", guidedMode ? "true" : "false"},
        {"mission_id", raid.missionId},
        {"profile_id", m_profile.profileId}
      }});

    raid.currentRoomId = "room.raid.patrol_hall";
    ++raid.roomsVisited;
    auto companionDecision = Peter::AI::EvaluateCompanion(
      state.companionConfig,
      Peter::AI::CompanionWorldContext{true, true, false, false, false, false, false, 5});
    EmitCompanionDecision(raid.currentRoomId, companionDecision);
    const auto patrolEncounter = Peter::Combat::ResolveEncounter(
      Peter::Combat::EncounterRequest{
        m_raidZone.encounters.at(0).enemies,
        companionDecision,
        false,
        false,
        raid.playerHealth});
    raid.playerHealth -= patrolEncounter.playerDamage;

    m_platform.ui->PresentPrompt("Loot the salvage cache in the side room.");
    const bool addedFirstScrap = Peter::Inventory::TryAddCarriedItem(
      state.inventory,
      state.loadout,
      "item.salvage.scrap_metal",
      2,
      nullptr);
    (void)addedFirstScrap;
    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::Gameplay,
      "gameplay.loot.recovered",
      {
        {"item_id", "item.salvage.scrap_metal"},
        {"quantity", "2"},
        {"room_id", "room.raid.salvage_nook"}
      }});

    raid.currentRoomId = "room.raid.guard_post";
    ++raid.roomsVisited;
    companionDecision = Peter::AI::EvaluateCompanion(
      state.companionConfig,
      Peter::AI::CompanionWorldContext{true, true, false, false, false, false, false, 6});
    EmitCompanionDecision(raid.currentRoomId, companionDecision);
    const auto guardEncounter = Peter::Combat::ResolveEncounter(
      Peter::Combat::EncounterRequest{
        m_raidZone.encounters.at(1).enemies,
        companionDecision,
        false,
        false,
        raid.playerHealth});
    raid.playerHealth -= guardEncounter.playerDamage;

    raid.currentRoomId = "room.raid.high_risk_vault";
    ++raid.roomsVisited;
    raid.highRiskRoomVisited = true;
    const bool addedVaultScrap = Peter::Inventory::TryAddCarriedItem(
      state.inventory,
      state.loadout,
      "item.salvage.scrap_metal",
      2,
      nullptr);
    const bool addedPowerCell = Peter::Inventory::TryAddCarriedItem(
      state.inventory,
      state.loadout,
      "item.salvage.power_cell",
      1,
      nullptr);
    (void)addedVaultScrap;
    (void)addedPowerCell;
    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::Gameplay,
      "gameplay.loot.recovered",
      {
        {"item_id", "item.salvage.power_cell"},
        {"quantity", "1"},
        {"room_id", raid.currentRoomId}
      }});

    raid.currentRoomId = m_raidZone.extractionRoomId;
    ++raid.roomsVisited;
    const auto extractionValidation =
      Peter::Validation::ValidateExtractionCountdown(m_raidZone.extraction.countdownSeconds);
    companionDecision = Peter::AI::EvaluateCompanion(
      state.companionConfig,
      Peter::AI::CompanionWorldContext{false, false, true, raid.playerHealth < 35, false, false, false, 4});
    EmitCompanionDecision(raid.currentRoomId, companionDecision);
    m_platform.audio->PostWorldCue(m_raidZone.extraction.successCueId);
    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::Gameplay,
      "gameplay.extraction.success",
      {
        {"countdown_seconds", std::to_string(m_raidZone.extraction.countdownSeconds)},
        {"validation", extractionValidation.valid ? "valid" : "invalid"},
        {"mission_id", raid.missionId}
      }});

    const auto extractedLoot = Peter::Inventory::MoveCarriedToStash(state.inventory);
    Peter::Core::ExtractionResult extractionResult;
    extractionResult.success = true;
    extractionResult.reason = "player_extracted_successfully";
    extractionResult.extractedItems = ToSliceLedger(extractedLoot);

    raid.success = true;
    state.completedRaids += 1;
    state.lastRaidResult = "success";

    const auto resultsScene = sceneShell.LoadRaidResults(raid.missionId, true);
    (void)resultsScene;
    m_platform.ui->PresentPanel(
      "raid.summary",
      Peter::UI::RenderPostRaidSummary(Peter::UI::PostRaidSummaryModel{
        true,
        Peter::Inventory::SummarizeLedger(extractedLoot),
        "none",
        companionDecision.topReason,
        "Use the workbench to craft the Salvage Pouch."}));

    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::Gameplay,
      "gameplay.raid.end",
      {
        {"mission_id", raid.missionId},
        {"player_health", std::to_string(raid.playerHealth)},
        {"result", "success"}
      }});

    const auto& workbench = FindStation("station.home.workbench");
    VisitStation(workbench);
    const auto craftRecipe = Peter::Progression::BuildPhase1SalvagePouchRecipe();
    const auto craftResult =
      Peter::Progression::CraftRecipe(craftRecipe, state.inventory, state.loadout, state.workshop);
    m_platform.ui->PresentPanel("workbench.craft_result", craftResult.summary);
    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::Gameplay,
      "gameplay.crafting.used",
      {
        {"crafted", craftResult.crafted ? "true" : "false"},
        {"recipe_id", craftRecipe.recipeId}
      }});

    const auto& companionStation = FindStation("station.home.companion");
    VisitStation(companionStation);
    m_platform.ui->PresentPanel("companion.explain", Peter::UI::RenderCompanionExplainPanel(companionDecision));
    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::CreatorTools,
      "creator_tools.explain_panel.opened",
      {
        {"state", companionDecision.currentState},
        {"action", companionDecision.lastAction}
      }});

    const auto previousConfig = state.companionConfig;
    const auto preview = Peter::Workshop::BuildFollowDistancePreview(
      state.companionConfig.followDistanceMeters,
      9.0);
    m_platform.ui->PresentPanel("companion.rule_preview", preview.summary + "\n" + preview.deltaSummary);
    if (preview.valid)
    {
      state.companionConfig.followDistanceMeters = preview.previewValue;
    }

    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::CreatorTools,
      "creator_tools.behavior_edit.applied",
      {
        {"preview_valid", preview.valid ? "true" : "false"},
        {"rule_id", preview.ruleId},
        {"value", std::to_string(preview.previewValue)}
      }});

    const auto beforePreview = Peter::AI::EvaluateCompanion(
      previousConfig,
      Peter::AI::CompanionWorldContext{false, false, false, false, false, false, true, 8});
    const auto afterPreview = Peter::AI::EvaluateCompanion(
      state.companionConfig,
      Peter::AI::CompanionWorldContext{false, false, false, false, false, false, true, 8});
    m_platform.ui->PresentPanel(
      "companion.rule_effect",
      std::string("Before: ") + beforePreview.currentState +
        "\nAfter: " + afterPreview.currentState +
        "\nChange: the companion can stay useful while the player is farther away.");
    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::CreatorTools,
      "creator_tools.behavior_edit.visible_next_raid",
      {
        {"after_state", afterPreview.currentState},
        {"before_state", beforePreview.currentState},
        {"rule_id", preview.ruleId}
      }});

    if (guidedMode)
    {
      state.guidedFirstRunComplete = true;
      state.ruleEditComplete = true;
      m_eventBus.Emit(Peter::Core::Event{
        Peter::Core::EventCategory::Gameplay,
        "gameplay.tutorial.completed",
        {{"tutorial_id", "tutorial.vertical_slice.guided_first_run"}}});
    }

    SavePersistentState(state);

    return SliceRunReport{
      true,
      "Completed the vertical slice loop: raid success, crafted Salvage Pouch, explained companion behavior, and applied a follow-distance edit.",
      companionDecision,
      preview,
      extractionResult};
  }

  SliceRunReport Phase1Slice::RunFailureSlice(PersistentState& state)
  {
    Peter::World::SceneShell sceneShell(m_eventBus);
    const auto homeScene = sceneShell.LoadHomeBase(m_homeBase);
    (void)homeScene;
    VisitStation(FindStation("station.home.mission_board"));
    const auto raidScene = sceneShell.LoadRaidZone(m_raidZone);
    (void)raidScene;
    m_platform.ui->PresentState("ui.raid_hud");

    Peter::Core::RaidSessionState raid;
    raid.missionId = m_raidZone.missionId;
    raid.sceneId = m_raidZone.sceneId;
    raid.currentRoomId = m_raidZone.entryRoomId;
    raid.playerHealth = 18;

    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::Gameplay,
      "gameplay.raid.start",
      {
        {"guided", "false"},
        {"mission_id", raid.missionId},
        {"profile_id", m_profile.profileId}
      }});

    const bool addedFailureScrap = Peter::Inventory::TryAddCarriedItem(
      state.inventory,
      state.loadout,
      "item.salvage.scrap_metal",
      2,
      nullptr);
    (void)addedFailureScrap;

    raid.currentRoomId = "room.raid.guard_post";
    auto companionDecision = Peter::AI::EvaluateCompanion(
      state.companionConfig,
      Peter::AI::CompanionWorldContext{true, false, false, false, false, false, false, 8});
    EmitCompanionDecision(raid.currentRoomId, companionDecision);
    const auto guardEncounter = Peter::Combat::ResolveEncounter(
      Peter::Combat::EncounterRequest{
        m_raidZone.encounters.at(1).enemies,
        companionDecision,
        false,
        false,
        raid.playerHealth});
    raid.playerHealth -= guardEncounter.playerDamage;

    raid.currentRoomId = "room.raid.high_risk_vault";
    raid.highRiskRoomVisited = true;
    companionDecision = Peter::AI::EvaluateCompanion(
      state.companionConfig,
      Peter::AI::CompanionWorldContext{true, false, false, true, false, true, false, 9});
    EmitCompanionDecision(raid.currentRoomId, companionDecision);
    const auto finalEncounter = Peter::Combat::ResolveEncounter(
      Peter::Combat::EncounterRequest{
        m_raidZone.encounters.at(2).enemies,
        companionDecision,
        true,
        true,
        raid.playerHealth});
    raid.playerHealth -= finalEncounter.playerDamage;
    raid.failed = finalEncounter.playerDefeated || raid.playerHealth <= 0;

    const auto lostLoot = Peter::Inventory::LoseCarried(state.inventory);
    Peter::Core::ExtractionResult extractionResult;
    extractionResult.success = false;
    extractionResult.reason = finalEncounter.causeOfFailure.empty()
      ? "failed_before_extraction"
      : finalEncounter.causeOfFailure;
    extractionResult.lostItems = ToSliceLedger(lostLoot);
    state.lastRaidResult = "failure";

    const auto resultsScene = sceneShell.LoadRaidResults(raid.missionId, false);
    (void)resultsScene;
    m_platform.audio->PostWorldCue(m_raidZone.extraction.failCueId);
    m_platform.ui->PresentPanel(
      "raid.summary",
      Peter::UI::RenderPostRaidSummary(Peter::UI::PostRaidSummaryModel{
        false,
        "none",
        Peter::Inventory::SummarizeLedger(lostLoot),
        companionDecision.topReason,
        "Try again, or adjust the companion before your next launch."}));

    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::Gameplay,
      "gameplay.extraction.failure",
      {
        {"cause", extractionResult.reason},
        {"lost_items", Peter::Inventory::SummarizeLedger(lostLoot)},
        {"mission_id", raid.missionId}
      }});
    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::Gameplay,
      "gameplay.raid.end",
      {
        {"cause_of_failure", extractionResult.reason},
        {"mission_id", raid.missionId},
        {"result", "failure"}
      }});

    SavePersistentState(state);

    return SliceRunReport{
      false,
      "Failure path completed: carried loot was lost, permanent upgrades remained untouched, and the raid summary explained the setback.",
      companionDecision,
      {},
      extractionResult};
  }
} // namespace Peter::App
