#include "Phase1Slice.h"

#include "PeterCombat/EncounterSimulator.h"
#include "PeterValidation/ValidationModule.h"
#include "PeterWorld/SceneShell.h"

#include <algorithm>
#include <sstream>
#include <stdexcept>

namespace Peter::App
{
  namespace
  {
    constexpr std::string_view kProfileMetaDomain = "save_domain.profile_meta";
    constexpr std::string_view kInventoryDomain = "save_domain.inventory";
    constexpr std::string_view kRecoveryDomain = "save_domain.recovery_state";
    constexpr std::string_view kWorkshopDomain = "save_domain.workshop_upgrades";
    constexpr std::string_view kMissionDomain = "save_domain.mission_progress";
    constexpr std::string_view kTutorialDomain = "save_domain.tutorial_progress";
    constexpr std::string_view kCompanionDomain = "save_domain.companion_config";
    constexpr std::string_view kAccessibilityDomain = "save_domain.settings_accessibility";

    Peter::Core::SliceItemLedger ToSliceLedger(const Peter::Inventory::ItemLedger& ledger)
    {
      Peter::Core::SliceItemLedger copy;
      for (const auto& [itemId, quantity] : ledger)
      {
        copy[itemId] = quantity;
      }
      return copy;
    }

    std::string JoinStrings(const std::vector<std::string>& values)
    {
      std::ostringstream output;
      bool first = true;
      for (const auto& value : values)
      {
        if (!first)
        {
          output << ",";
        }
        output << value;
        first = false;
      }
      return output.str();
    }

    std::vector<std::string> SplitStrings(const std::string_view serialized)
    {
      std::vector<std::string> values;
      std::size_t cursor = 0;
      while (cursor < serialized.size())
      {
        const auto separator = serialized.find(',', cursor);
        const auto token = serialized.substr(
          cursor,
          separator == std::string_view::npos ? serialized.size() - cursor : separator - cursor);
        if (!token.empty())
        {
          values.emplace_back(token);
        }
        if (separator == std::string_view::npos)
        {
          break;
        }
        cursor = separator + 1;
      }
      return values;
    }

    bool HasLesson(const std::vector<std::string>& completedLessons, const std::string_view lessonId)
    {
      return std::find(completedLessons.begin(), completedLessons.end(), lessonId) != completedLessons.end();
    }

    bool HasNode(const Peter::Progression::WorkshopState& workshopState, const std::string_view nodeId)
    {
      return Peter::Progression::HasUnlockedNode(workshopState, nodeId);
    }

    std::string FirstLockedNodeForTrack(
      const Peter::Progression::WorkshopState& workshopState,
      const std::string_view trackId)
    {
      const auto* track = Peter::Progression::FindUpgradeTrack(trackId);
      if (track == nullptr)
      {
        return {};
      }

      for (const auto& node : track->nodes)
      {
        if (!Peter::Progression::HasUnlockedNode(workshopState, node.nodeId))
        {
          return node.nodeId;
        }
      }

      return {};
    }

    std::string NextUpgradeNodeForMission(
      const Peter::Progression::WorkshopState& workshopState,
      const Peter::World::MissionTemplateDefinition& mission)
    {
      if (mission.templateType == "salvage_run")
      {
        return FirstLockedNodeForTrack(workshopState, "track.inventory_capacity");
      }
      if (mission.templateType == "recover_artifact" || mission.templateType == "activate_machine")
      {
        return FirstLockedNodeForTrack(workshopState, "track.player_tools");
      }
      if (mission.templateType == "escort_companion")
      {
        return FirstLockedNodeForTrack(workshopState, "track.companion_capabilities");
      }
      if (mission.templateType == "timed_extraction")
      {
        return FirstLockedNodeForTrack(workshopState, "track.creator_unlocks");
      }
      return {};
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
    if (scenarioName == "artifact_recovery")
    {
      return SliceScenario::ArtifactRecovery;
    }
    if (scenarioName == "escort_support")
    {
      return SliceScenario::EscortSupport;
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
      case SliceScenario::ArtifactRecovery:
        return "artifact_recovery";
      case SliceScenario::EscortSupport:
        return "escort_support";
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
    const auto defaultBindings = m_platform.input->DefaultBindings();
    state.accessibility = Peter::UI::AccessibilitySettingsFromSaveFields({}, defaultBindings);

    if (m_saveDomainStore.DomainExists(kInventoryDomain))
    {
      const auto fields = m_saveDomainStore.ReadDomain(std::string(kInventoryDomain));
      if (fields.find("schema_version") == fields.end())
      {
        m_eventBus.Emit(Peter::Core::Event{
          Peter::Core::EventCategory::SaveLoad,
          "save_load.migration.phase1_to_phase2",
          {{"domain_id", std::string(kInventoryDomain)}, {"profile_id", m_profile.profileId}}});
      }
      Peter::Inventory::LoadFromSaveFields(fields, state.inventory, state.loadout);
    }
    else
    {
      state.inventory.carrySlotCapacity = Peter::Inventory::MaxCarrySlots(state.loadout);
      for (const auto& itemId : Peter::Inventory::EquippedItemIds(state.loadout))
      {
        const auto* definition = Peter::Inventory::FindItemDefinition(itemId);
        if (definition != nullptr && definition->repairable)
        {
          state.inventory.equippedDurability[itemId] = definition->maxDurability;
        }
      }
    }

    if (m_saveDomainStore.DomainExists(kRecoveryDomain))
    {
      Peter::Inventory::LoadRecoveryStateFromSaveFields(
        m_saveDomainStore.ReadDomain(std::string(kRecoveryDomain)),
        state.recovery);
    }

    if (m_saveDomainStore.DomainExists(kWorkshopDomain))
    {
      const auto fields = m_saveDomainStore.ReadDomain(std::string(kWorkshopDomain));
      if (fields.find("schema_version") == fields.end())
      {
        m_eventBus.Emit(Peter::Core::Event{
          Peter::Core::EventCategory::SaveLoad,
          "save_load.migration.phase1_to_phase2",
          {{"domain_id", std::string(kWorkshopDomain)}, {"profile_id", m_profile.profileId}}});
      }
      state.workshop = Peter::Progression::WorkshopStateFromSaveFields(fields);
    }

    if (m_saveDomainStore.DomainExists(kCompanionDomain))
    {
      state.companionConfig = Peter::AI::CompanionConfigFromSaveFields(
        m_saveDomainStore.ReadDomain(std::string(kCompanionDomain)));
    }

    if (m_saveDomainStore.DomainExists(kAccessibilityDomain))
    {
      state.accessibility = Peter::UI::AccessibilitySettingsFromSaveFields(
        m_saveDomainStore.ReadDomain(std::string(kAccessibilityDomain)),
        defaultBindings);
    }

    if (m_saveDomainStore.DomainExists(kProfileMetaDomain))
    {
      const auto fields = m_saveDomainStore.ReadDomain(std::string(kProfileMetaDomain));
      const auto completedRaids = fields.find("completed_raids");
      const auto lastResult = fields.find("last_raid_result");
      state.completedRaids = completedRaids == fields.end() ? 0 : std::stoi(completedRaids->second);
      state.lastRaidResult = lastResult == fields.end() ? "none" : lastResult->second;
    }

    if (m_saveDomainStore.DomainExists(kMissionDomain))
    {
      const auto fields = m_saveDomainStore.ReadDomain(std::string(kMissionDomain));
      const auto lastMission = fields.find("last_mission_id");
      state.lastMissionId = lastMission == fields.end() ? state.lastMissionId : lastMission->second;
    }

    if (m_saveDomainStore.DomainExists(kTutorialDomain))
    {
      const auto fields = m_saveDomainStore.ReadDomain(std::string(kTutorialDomain));
      const auto guided = fields.find("guided_first_run_complete");
      const auto ruleEdit = fields.find("rule_edit_complete");
      const auto hintLevel = fields.find("tutorial_hint_level");
      const auto completedLessons = fields.find("completed_lessons");
      state.guidedFirstRunComplete = guided != fields.end() && guided->second == "true";
      state.ruleEditComplete = ruleEdit != fields.end() && ruleEdit->second == "true";
      state.tutorialHintLevel = hintLevel == fields.end() ? 0 : std::stoi(hintLevel->second);
      state.completedLessons =
        completedLessons == fields.end() ? std::vector<std::string>{} : SplitStrings(completedLessons->second);
    }

    const auto favoriteValidation =
      Peter::Validation::ValidateFavoriteItemReference(state.loadout.favoriteItemId, state.inventory);
    if (!favoriteValidation.valid || state.loadout.favoriteItemId.empty())
    {
      state.loadout.favoriteItemId = state.loadout.equippedToolId;
    }

    return state;
  }

  void Phase1Slice::SavePersistentState(const PersistentState& state) const
  {
    m_saveDomainStore.WriteDomain(
      std::string(kInventoryDomain),
      Peter::Inventory::ToSaveFields(state.inventory, state.loadout));
    m_saveDomainStore.WriteDomain(
      std::string(kRecoveryDomain),
      Peter::Inventory::RecoveryStateToSaveFields(state.recovery));
    m_saveDomainStore.WriteDomain(
      std::string(kWorkshopDomain),
      Peter::Progression::ToSaveFields(state.workshop));
    m_saveDomainStore.WriteDomain(
      std::string(kCompanionDomain),
      Peter::AI::ToSaveFields(state.companionConfig));
    m_saveDomainStore.WriteDomain(
      std::string(kAccessibilityDomain),
      Peter::UI::ToSaveFields(state.accessibility));
    m_saveDomainStore.WriteDomain(
      std::string(kProfileMetaDomain),
      Peter::Core::StructuredFields{
        {"schema_version", "2"},
        {"completed_raids", std::to_string(state.completedRaids)},
        {"last_raid_result", state.lastRaidResult},
        {"profile_id", m_profile.profileId}
      });
    m_saveDomainStore.WriteDomain(
      std::string(kTutorialDomain),
      Peter::Core::StructuredFields{
        {"schema_version", "2"},
        {"completed_lessons", JoinStrings(state.completedLessons)},
        {"guided_first_run_complete", state.guidedFirstRunComplete ? "true" : "false"},
        {"rule_edit_complete", state.ruleEditComplete ? "true" : "false"},
        {"tutorial_hint_level", std::to_string(state.tutorialHintLevel)}
      });
    m_saveDomainStore.WriteDomain(
      std::string(kMissionDomain),
      Peter::Core::StructuredFields{
        {"schema_version", "2"},
        {"last_mission_id", state.lastMissionId},
        {"last_scene_id", m_raidZone.sceneId},
        {"last_result", state.lastRaidResult}
      });
  }

  void Phase1Slice::PresentHomeBase(const PersistentState& state) const
  {
    auto cameraRig = m_traversal.cameraRig;
    cameraRig.followDistanceMeters =
      std::max(cameraRig.followDistanceMeters, state.companionConfig.followDistanceMeters - 1.0);
    m_platform.camera->ApplyRig(cameraRig);
    m_platform.ui->ApplyPresentationSettings(Peter::Adapters::PresentationSettings{
      state.accessibility.subtitlesEnabled,
      state.accessibility.subtitleScalePercent,
      state.accessibility.textScalePercent});
    m_platform.ui->PresentState("ui.home_base");
    m_platform.ui->PresentPanel("home_base.overview", Peter::UI::RenderHomeBaseOverview(m_homeBase));
    m_platform.ui->PresentPanel(
      "home_base.accessibility",
      Peter::UI::RenderAccessibilitySettings(state.accessibility, m_platform.input->DefaultBindings()));
    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::Gameplay,
      "gameplay.traversal.ready",
      {
        {"camera_mode", cameraRig.mode},
        {"input_scheme", m_platform.input->ActiveScheme()},
        {"interaction_range_meters", std::to_string(m_traversal.interactionRangeMeters)},
        {"reduced_time_pressure", state.accessibility.reducedTimePressure ? "true" : "false"}
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
        {"action", snapshot.lastAction},
        {"reason", snapshot.topReason},
        {"room_id", std::string(roomId)},
        {"state", snapshot.currentState}
      }});
  }

  void Phase1Slice::RunLesson(PersistentState& state, const std::string_view lessonId, const bool replayed) const
  {
    const auto* lesson = Peter::World::FindTutorialLesson(lessonId);
    if (lesson == nullptr)
    {
      return;
    }

    if (replayed)
    {
      state.tutorialHintLevel = std::max(1, state.tutorialHintLevel + 1);
      m_eventBus.Emit(Peter::Core::Event{
        Peter::Core::EventCategory::Gameplay,
        "gameplay.tutorial.replayed",
        {{"hint_level", std::to_string(state.tutorialHintLevel)}, {"lesson_id", std::string(lessonId)}}});
    }

    m_platform.ui->PresentPanel(
      "tutorial.lesson",
      Peter::UI::RenderTutorialLesson(*lesson, state.tutorialHintLevel));

    for (const auto& step : lesson->steps)
    {
      if (step.hintThreshold > 0 && state.tutorialHintLevel < step.hintThreshold)
      {
        state.tutorialHintLevel = step.hintThreshold;
        m_eventBus.Emit(Peter::Core::Event{
          Peter::Core::EventCategory::Gameplay,
          "gameplay.tutorial.hint_escalated",
          {{"hint_level", std::to_string(state.tutorialHintLevel)}, {"lesson_id", std::string(lessonId)}}});
      }

      m_platform.ui->PresentPrompt(step.prompt);
      m_eventBus.Emit(Peter::Core::Event{
        Peter::Core::EventCategory::Gameplay,
        "gameplay.tutorial.step.presented",
        {
          {"action", step.action},
          {"lesson_id", std::string(lessonId)},
          {"step_id", step.id},
          {"target_id", step.targetId}
        }});
    }

    if (!HasLesson(state.completedLessons, lessonId))
    {
      state.completedLessons.push_back(std::string(lessonId));
    }
  }

  SliceRunReport Phase1Slice::Run(SliceScenario scenario)
  {
    auto state = LoadPersistentState();
    PresentHomeBase(state);

    switch (scenario)
    {
      case SliceScenario::GuidedFirstRun:
        return RunMissionScenario(state, "mission.salvage_run.machine_silo", true, true);
      case SliceScenario::HappyPath:
      case SliceScenario::Smoke:
        return RunMissionScenario(state, "mission.salvage_run.machine_silo", true, false);
      case SliceScenario::FailurePath:
        return RunMissionScenario(state, "mission.timed_extraction.machine_silo", false, false);
      case SliceScenario::ArtifactRecovery:
        return RunMissionScenario(state, "mission.recover_artifact.machine_silo", true, false);
      case SliceScenario::EscortSupport:
        return RunMissionScenario(state, "mission.escort_companion.machine_silo", true, false);
    }

    return RunMissionScenario(state, "mission.salvage_run.machine_silo", true, false);
  }

  SliceRunReport Phase1Slice::RunMissionScenario(
    PersistentState& state,
    const std::string_view missionId,
    const bool expectSuccess,
    const bool guidedMode)
  {
    Peter::World::SceneShell sceneShell(m_eventBus);
    const auto homeScene = sceneShell.LoadHomeBase(m_homeBase);
    (void)homeScene;

    const auto* mission = Peter::World::FindMissionTemplate(missionId);
    if (mission == nullptr)
    {
      throw std::runtime_error("Missing mission template.");
    }

    VisitStation(FindStation("station.home.mission_board"));
    m_platform.ui->PresentPanel(
      "mission.board",
      Peter::UI::RenderMissionBoard(Peter::World::BuildPhase2MissionTemplates(), mission->id));
    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::Gameplay,
      "gameplay.mission.selected",
      {
        {"mission_id", mission->id},
        {"template", mission->templateType}
      }});

    const auto missionValidation = Peter::Validation::ValidateMissionTemplate(*mission);
    const auto extractionValidation = Peter::Validation::ValidateExtractionCountdown(
      state.accessibility.reducedTimePressure
        ? mission->extractionCountdownSeconds + (mission->extractionCountdownSeconds / 2)
        : mission->extractionCountdownSeconds);
    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::Validation,
      "validation.runtime.mission_template",
      {
        {"message", missionValidation.message},
        {"mission_id", mission->id},
        {"valid", missionValidation.valid ? "true" : "false"}
      }});
    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::Validation,
      "validation.runtime.extraction",
      {
        {"message", extractionValidation.message},
        {"mission_id", mission->id},
        {"valid", extractionValidation.valid ? "true" : "false"}
      }});

    if (guidedMode)
    {
      RunLesson(state, "lesson.phase2.mission_choice", false);
    }

    const auto raidScene = sceneShell.LoadRaidZone(m_raidZone);
    (void)raidScene;
    m_platform.ui->PresentState("ui.raid_hud");
    m_platform.ui->PresentPanel("raid.overview", Peter::UI::RenderRaidZoneOverview(m_raidZone, *mission));

    Peter::Core::RaidSessionState raid;
    raid.missionId = mission->id;
    raid.missionTemplateId = mission->templateType;
    raid.sceneId = m_raidZone.sceneId;
    raid.currentRoomId = m_raidZone.entryRoomId;
    raid.playerHealth = expectSuccess ? 100 : 26;
    raid.reducedTimePressure = state.accessibility.reducedTimePressure;
    raid.timeline.push_back("Mission selected: " + mission->displayName);

    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::Gameplay,
      "gameplay.raid.start",
      {
        {"guided", guidedMode ? "true" : "false"},
        {"mission_id", raid.missionId},
        {"profile_id", m_profile.profileId},
        {"template", raid.missionTemplateId}
      }});

    if (guidedMode && !HasLesson(state.completedLessons, "lesson.phase2.first_combat"))
    {
      RunLesson(state, "lesson.phase2.first_combat", false);
    }

    const auto inputValidation =
      Peter::Validation::ValidateInputBindings(m_platform.input->DefaultBindings(), state.accessibility);
    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::Validation,
      "validation.runtime.input_bindings",
      {
        {"message", inputValidation.message},
        {"valid", inputValidation.valid ? "true" : "false"}
      }});

    Peter::AI::CompanionWorldContext combatContext;
    combatContext.threatVisible = true;
    combatContext.sameTargetMarked = true;
    combatContext.playerLowHealth = !expectSuccess;
    combatContext.unsafeToAdvance = mission->templateType == "escort_companion";
    combatContext.rareLootVisible = mission->templateType == "salvage_run" || mission->templateType == "recover_artifact";
    combatContext.timedMissionPressure = mission->templateType == "timed_extraction";
    combatContext.guardProtocolUnlocked = HasNode(state.workshop, "track.companion_capabilities.guard_protocol");
    combatContext.repairPulseUnlocked = HasNode(state.workshop, "track.companion_capabilities.repair_pulse");
    combatContext.lootPingUnlocked = HasNode(state.workshop, "track.companion_capabilities.loot_ping");
    combatContext.distanceToPlayerMeters = mission->templateType == "escort_companion" ? 4 : 6;

    auto companionDecision = Peter::AI::EvaluateCompanion(state.companionConfig, combatContext);
    EmitCompanionDecision("room.raid.patrol_hall", companionDecision);

    Peter::Combat::EncounterRequest encounterRequest{
      m_raidZone.encounters.at(0).enemies,
      companionDecision,
      false,
      mission->templateType == "recover_artifact" || mission->templateType == "timed_extraction",
      mission->templateType == "timed_extraction",
      raid.playerHealth};
    auto encounterOutcome = Peter::Combat::ResolveEncounter(encounterRequest);
    for (const auto& event : encounterOutcome.events)
    {
      m_eventBus.Emit(event);
    }
    raid.playerHealth = std::max(0, raid.playerHealth - encounterOutcome.playerDamage + encounterOutcome.playerHealing);
    raid.timeline.push_back("Combat: " + encounterOutcome.summary);

    if (mission->templateType == "timed_extraction" || mission->templateType == "escort_companion")
    {
      combatContext.playerLowHealth = !expectSuccess;
      combatContext.playerNeedsRevive = mission->templateType == "escort_companion" &&
        HasNode(state.workshop, "track.companion_capabilities.repair_pulse");
      combatContext.distanceToPlayerMeters = 3;
      companionDecision = Peter::AI::EvaluateCompanion(state.companionConfig, combatContext);
      EmitCompanionDecision("room.raid.guard_post", companionDecision);
      encounterRequest.enemies = m_raidZone.encounters.at(1).enemies;
      encounterRequest.companionDecision = companionDecision;
      encounterRequest.playerHealth = raid.playerHealth;
      encounterRequest.playerChoosesStealth = mission->templateType == "timed_extraction";
      encounterRequest.highRiskRoom = mission->templateType != "escort_companion";
      encounterRequest.timedMissionPressure = mission->templateType == "timed_extraction";
      encounterOutcome = Peter::Combat::ResolveEncounter(encounterRequest);
      for (const auto& event : encounterOutcome.events)
      {
        m_eventBus.Emit(event);
      }
      raid.playerHealth = std::max(0, raid.playerHealth - encounterOutcome.playerDamage + encounterOutcome.playerHealing);
      raid.timeline.push_back("Second combat: " + encounterOutcome.summary);
    }

    for (const auto& objective : mission->objectives)
    {
      if (objective.kind == "collect_item")
      {
        std::string denialReason;
        const bool addedObjectiveItem = Peter::Inventory::TryAddCarriedItem(
          state.inventory,
          state.loadout,
          objective.targetId,
          objective.requiredCount,
          &denialReason);
        (void)addedObjectiveItem;
        if (const auto* definition = Peter::Inventory::FindItemDefinition(objective.targetId))
        {
          m_platform.audio->PostFeedbackCue(definition->audioCueFamily, std::string(Peter::Inventory::ToString(definition->rarity)));
          m_platform.ui->PresentPrompt(Peter::Inventory::RenderRarityFeedback(*definition));
          m_eventBus.Emit(Peter::Core::Event{
            Peter::Core::EventCategory::Gameplay,
            "gameplay.loot.recovered",
            {
              {"category", std::string(Peter::Inventory::ToString(definition->category))},
              {"item_id", objective.targetId},
              {"quantity", std::to_string(objective.requiredCount)},
              {"rarity", std::string(Peter::Inventory::ToString(definition->rarity))}
            }});
        }
      }
      else if (objective.kind == "activate_target")
      {
        m_eventBus.Emit(Peter::Core::Event{
          Peter::Core::EventCategory::Gameplay,
          "gameplay.objective.activated",
          {
            {"objective_id", objective.id},
            {"target_id", objective.targetId}
          }});
      }
      else if (objective.kind == "repair_target")
      {
        m_eventBus.Emit(Peter::Core::Event{
          Peter::Core::EventCategory::Gameplay,
          "gameplay.objective.repaired",
          {
            {"objective_id", objective.id},
            {"tool_id", state.loadout.equippedToolId}
          }});
      }
      else if (objective.kind == "escort")
      {
        m_eventBus.Emit(Peter::Core::Event{
          Peter::Core::EventCategory::Gameplay,
          "gameplay.escort.started",
          {{"objective_id", objective.id}}});
      }

      if (objective.kind != "extract")
      {
        raid.timeline.push_back("Objective: " + objective.description);
      }
    }

    for (const auto& sideObjective : mission->sideObjectives)
    {
      if (!expectSuccess)
      {
        continue;
      }

      if (sideObjective.kind == "collect_item")
      {
        const bool addedSideObjectiveItem = Peter::Inventory::TryAddCarriedItem(
          state.inventory,
          state.loadout,
          sideObjective.targetId,
          sideObjective.requiredCount,
          nullptr);
        (void)addedSideObjectiveItem;
      }
      m_eventBus.Emit(Peter::Core::Event{
        Peter::Core::EventCategory::Gameplay,
        "gameplay.mission.side_objective.completed",
        {
          {"mission_id", mission->id},
          {"objective_id", sideObjective.id}
        }});
      raid.timeline.push_back("Side objective: " + sideObjective.description);
    }

    Peter::Core::ExtractionResult extractionResult;
    Peter::World::RaidSummary raidSummary;
    raidSummary.missionId = mission->id;
    raidSummary.missionDisplayName = mission->displayName;
    raidSummary.timeline = raid.timeline;
    raidSummary.companionHighlight = companionDecision.topReason;
    raidSummary.lessonTip = mission->rewardBundle.lessonTipId;

    if (expectSuccess)
    {
      const int countdownSeconds = state.accessibility.reducedTimePressure
        ? mission->extractionCountdownSeconds + (mission->extractionCountdownSeconds / 2)
        : mission->extractionCountdownSeconds;
      combatContext.threatVisible = false;
      combatContext.sameTargetMarked = false;
      combatContext.extractionActive = true;
      combatContext.playerLowHealth = raid.playerHealth < 35;
      companionDecision = Peter::AI::EvaluateCompanion(state.companionConfig, combatContext);
      EmitCompanionDecision(m_raidZone.extractionRoomId, companionDecision);

      m_platform.audio->PostWorldCue(m_raidZone.extraction.successCueId);
      m_eventBus.Emit(Peter::Core::Event{
        Peter::Core::EventCategory::Gameplay,
        "gameplay.extraction.success",
        {
          {"countdown_seconds", std::to_string(countdownSeconds)},
          {"mission_id", raid.missionId},
          {"reduced_time_pressure", state.accessibility.reducedTimePressure ? "true" : "false"}
        }});

      const auto extractedLoot = Peter::Inventory::MoveCarriedToStash(state.inventory);
      for (const auto& [itemId, quantity] : mission->rewardBundle.guaranteedItems)
      {
        Peter::Inventory::AddToLedger(state.inventory.stash, itemId, quantity);
      }

      extractionResult.success = true;
      extractionResult.reason = "player_extracted_successfully";
      extractionResult.extractedItems = ToSliceLedger(extractedLoot);
      raid.success = true;
      raid.timeline.push_back("Extraction succeeded.");
      raidSummary.success = true;
      raidSummary.gainedItems = Peter::Inventory::SummarizeLedger(extractedLoot);
      raidSummary.lostItems = "none";
      raidSummary.brokenItems = "none";
      raidSummary.recoveredItems = "none";

      VisitStation(FindStation("station.home.workbench"));
      std::string workshopPanel = Peter::UI::RenderWorkshopTracks(
        Peter::Progression::BuildPhase2UpgradeTracks(),
        state.workshop);
      const auto nextNode = NextUpgradeNodeForMission(state.workshop, *mission);
      if (!nextNode.empty())
      {
        const auto unlockResult =
          Peter::Progression::UnlockUpgradeNode(nextNode, state.inventory, state.loadout, state.workshop);
        workshopPanel += "\n" + unlockResult.summary;
        m_eventBus.Emit(Peter::Core::Event{
          Peter::Core::EventCategory::Gameplay,
          "gameplay.workshop.upgrade_unlocked",
          {
            {"next_reward", unlockResult.nextReward},
            {"node_id", nextNode},
            {"unlocked", unlockResult.unlocked ? "true" : "false"}
          }});
      }
      m_platform.ui->PresentPanel("workbench.tracks", workshopPanel);

      VisitStation(FindStation("station.home.companion"));
      m_platform.ui->PresentPanel("companion.explain", Peter::UI::RenderCompanionExplainPanel(companionDecision));
      m_eventBus.Emit(Peter::Core::Event{
        Peter::Core::EventCategory::CreatorTools,
        "creator_tools.explain_panel.opened",
        {
          {"action", companionDecision.lastAction},
          {"state", companionDecision.currentState}
        }});

      if (guidedMode)
      {
        const auto previousConfig = state.companionConfig;
        const auto preview = Peter::Workshop::BuildFollowDistancePreview(
          state.companionConfig.followDistanceMeters,
          9.0);
        m_platform.ui->PresentPanel("companion.rule_preview", preview.summary + "\n" + preview.deltaSummary);
        if (preview.valid)
        {
          state.companionConfig.followDistanceMeters = preview.previewValue;
        }
        const auto beforePreview = Peter::AI::EvaluateCompanion(
          previousConfig,
          Peter::AI::CompanionWorldContext{
            false,
            false,
            false,
            false,
            false,
            false,
            true,
            false,
            HasNode(state.workshop, "track.companion_capabilities.guard_protocol"),
            HasNode(state.workshop, "track.companion_capabilities.repair_pulse"),
            HasNode(state.workshop, "track.companion_capabilities.loot_ping"),
            8});
        const auto afterPreview = Peter::AI::EvaluateCompanion(
          state.companionConfig,
          Peter::AI::CompanionWorldContext{
            false,
            false,
            false,
            false,
            false,
            false,
            true,
            false,
            HasNode(state.workshop, "track.companion_capabilities.guard_protocol"),
            HasNode(state.workshop, "track.companion_capabilities.repair_pulse"),
            HasNode(state.workshop, "track.companion_capabilities.loot_ping"),
            8});
        m_platform.ui->PresentPanel(
          "companion.rule_effect",
          std::string("Before: ") + beforePreview.currentState +
            "\nAfter: " + afterPreview.currentState +
            "\nChange: the companion stays helpful from farther away.");
        state.ruleEditComplete = preview.valid;
      }

      if (guidedMode)
      {
        state.guidedFirstRunComplete = true;
      }

      state.completedRaids += 1;
      state.lastRaidResult = "success";
      state.lastMissionId = mission->id;
      const auto resultsScene = sceneShell.LoadRaidResults(raid.missionId, true);
      (void)resultsScene;
    }
    else
    {
      Peter::Inventory::AddToLedger(state.inventory.stash, "item.salvage.scrap_metal", 2);
      state.inventory.equippedDurability[state.loadout.equippedToolId] = 20;
      const auto lostLoot = Peter::Inventory::LoseCarried(state.inventory);
      const auto recoveryResolution = Peter::Inventory::ApplyRaidFailureRecovery(
        state.inventory,
        state.loadout,
        state.recovery,
        state.accessibility.reducedTimePressure);

      extractionResult.success = false;
      extractionResult.reason = mission->failRuleId == "fail_rule.timer_expired"
        ? "timer_expired_before_extraction"
        : "failed_before_extraction";
      extractionResult.lostItems = ToSliceLedger(lostLoot);
      extractionResult.brokenItems = ToSliceLedger(recoveryResolution.brokenItems);

      raid.failed = true;
      raid.timeline.push_back("Mission failed before extraction.");
      raidSummary.success = false;
      raidSummary.gainedItems = "none";
      raidSummary.lostItems = Peter::Inventory::SummarizeLedger(lostLoot);
      raidSummary.brokenItems = Peter::Inventory::SummarizeLedger(recoveryResolution.brokenItems);

      m_platform.audio->PostWorldCue(m_raidZone.extraction.failCueId);
      m_eventBus.Emit(Peter::Core::Event{
        Peter::Core::EventCategory::Gameplay,
        "gameplay.extraction.failure",
        {
          {"cause", extractionResult.reason},
          {"lost_items", raidSummary.lostItems},
          {"mission_id", raid.missionId}
        }});

      VisitStation(FindStation("station.home.workbench"));
      std::string recoverySummary;
      std::string repairSummary;
      bool recoveryUsed = Peter::Inventory::ReclaimFavoriteItem(
        state.recovery,
        state.inventory,
        state.loadout,
        &recoverySummary);
      if (!recoveryUsed && !state.recovery.brokenItems.empty())
      {
        const auto itemId = state.recovery.brokenItems.begin()->first;
        recoveryUsed = Peter::Inventory::RepairBrokenItem(state.recovery, state.inventory, itemId, &repairSummary);
      }
      raidSummary.recoveredItems = recoveryUsed ? (recoverySummary.empty() ? repairSummary : recoverySummary) : "none";
      if (recoveryUsed)
      {
        m_eventBus.Emit(Peter::Core::Event{
          Peter::Core::EventCategory::Gameplay,
          "gameplay.workshop.recovery_used",
          {
            {"mission_id", raid.missionId},
            {"result", raidSummary.recoveredItems}
          }});
      }

      RunLesson(state, "lesson.phase2.first_repair", true);
      state.lastRaidResult = "failure";
      state.lastMissionId = mission->id;
      const auto resultsScene = sceneShell.LoadRaidResults(raid.missionId, false);
      (void)resultsScene;
    }

    m_platform.ui->PresentPanel("raid.summary", Peter::UI::RenderPostRaidSummary(raidSummary));
    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::Gameplay,
      "gameplay.post_raid.summary_opened",
      {
        {"mission_id", raid.missionId},
        {"result", raidSummary.success ? "success" : "failure"}
      }});
    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::Gameplay,
      "gameplay.raid.end",
      {
        {"mission_id", raid.missionId},
        {"player_health", std::to_string(raid.playerHealth)},
        {"result", raidSummary.success ? "success" : "failure"}
      }});

    SavePersistentState(state);

    return SliceRunReport{
      raidSummary.success,
      raidSummary.success
        ? "Completed the Phase 2 systems alpha loop: mission, combat, progression, explainability, and post-raid clarity."
        : "Completed the Phase 2 failure-and-recovery loop: loss was explained, durability degraded fairly, and recovery remained available.",
      mission->id,
      companionDecision,
      Peter::Workshop::BuildFollowDistancePreview(
        state.companionConfig.followDistanceMeters,
        state.companionConfig.followDistanceMeters),
      extractionResult,
      raidSummary};
  }
} // namespace Peter::App
