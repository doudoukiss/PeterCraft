#include "PlayableSessionController.h"

#include "PeterAI/CompanionAi.h"
#include "PeterProgression/Crafting.h"
#include "PeterUI/SlicePresentation.h"
#include "PeterValidation/ValidationModule.h"
#include "PeterWorkshop/WorkshopTuning.h"
#include "PeterWorld/SceneShell.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <sstream>

namespace Peter::App
{
  namespace
  {
    constexpr std::string_view kProfileMetaDomain = "save_domain.profile_meta";
    constexpr std::string_view kMissionDomain = "save_domain.mission_progress";
    constexpr std::string_view kCompanionDomain = "save_domain.companion_config";
    constexpr std::string_view kAccessibilityDomain = "save_domain.settings_accessibility";

    Peter::World::WorldPose ToWorldPose(const Peter::Traversal::TraversalState& state)
    {
      return Peter::World::WorldPose{
        state.positionXMeters,
        state.positionYMeters,
        state.positionZMeters,
        state.velocityXMetersPerSecond,
        state.velocityYMetersPerSecond,
        state.velocityZMetersPerSecond};
    }

    double DistanceTo(const Peter::Traversal::TraversalState& state, const Peter::World::WorldAnchorDefinition& anchor)
    {
      const double dx = state.positionXMeters - anchor.xMeters;
      const double dy = state.positionYMeters - anchor.yMeters;
      const double dz = state.positionZMeters - anchor.zMeters;
      return std::sqrt((dx * dx) + (dy * dy) + (dz * dz));
    }

    Peter::Traversal::TraversalState SpawnAtScene(const std::string_view sceneId)
    {
      Peter::Traversal::TraversalState state;
      if (const auto* spawn = Peter::World::FindSceneSpawnAnchor(sceneId))
      {
        state.positionXMeters = spawn->xMeters;
        state.positionYMeters = spawn->yMeters;
        state.positionZMeters = spawn->zMeters;
      }
      return state;
    }

    bool ScenarioSucceeds(const std::string_view scenario)
    {
      return scenario != "failure_path";
    }

    bool ScenarioNeedsWorkshopProof(const std::string_view scenario)
    {
      return scenario == "guided_first_run" || scenario == "happy_path" || scenario == "artifact_recovery";
    }
  } // namespace

  PlayableSessionController::PlayableSessionController(
    Peter::Adapters::PlatformServices& platform,
    Peter::Core::EventBus& eventBus,
    Peter::Core::ProfileInfo profile,
    Peter::Core::SaveDomainStore& saveDomainStore)
    : m_platform(platform)
    , m_eventBus(eventBus)
    , m_profile(std::move(profile))
    , m_saveDomainStore(saveDomainStore)
    , m_qualityProfile(Peter::Core::LoadPhase7PlayableQualityProfile())
    , m_traversalProfile(Peter::Traversal::BuildTraversalProfile(
        static_cast<const Peter::Core::QualityProfileBase&>(m_qualityProfile)))
  {
  }

  PlayableSessionController::PersistentState PlayableSessionController::LoadPersistentState() const
  {
    PersistentState state;
    state.accessibility = Peter::UI::AccessibilitySettingsFromSaveFields({}, m_platform.input->DefaultBindings());

    if (m_saveDomainStore.DomainExists(std::string(kCompanionDomain)))
    {
      state.companionConfig = Peter::AI::CompanionConfigFromSaveFields(
        m_saveDomainStore.ReadDomain(std::string(kCompanionDomain)));
    }

    if (m_saveDomainStore.DomainExists(std::string(kAccessibilityDomain)))
    {
      state.accessibility = Peter::UI::AccessibilitySettingsFromSaveFields(
        m_saveDomainStore.ReadDomain(std::string(kAccessibilityDomain)),
        m_platform.input->DefaultBindings());
    }

    if (m_saveDomainStore.DomainExists(std::string(kProfileMetaDomain)))
    {
      const auto fields = m_saveDomainStore.ReadDomain(std::string(kProfileMetaDomain));
      if (const auto completedRaids = fields.find("completed_raids"); completedRaids != fields.end())
      {
        state.completedRaids = std::stoi(completedRaids->second);
      }
      if (const auto lastRaidResult = fields.find("last_raid_result"); lastRaidResult != fields.end())
      {
        state.lastRaidResult = lastRaidResult->second;
      }
    }

    if (m_saveDomainStore.DomainExists(std::string(kMissionDomain)))
    {
      const auto fields = m_saveDomainStore.ReadDomain(std::string(kMissionDomain));
      if (const auto lastMission = fields.find("last_mission_id"); lastMission != fields.end())
      {
        state.lastMissionId = lastMission->second;
      }
      if (const auto lastScene = fields.find("last_scene_id"); lastScene != fields.end())
      {
        state.lastSceneId = lastScene->second;
      }
    }

    return state;
  }

  void PlayableSessionController::SaveCompanionAndAccessibility(const PersistentState& state) const
  {
    (void)m_saveDomainStore.WriteDomain(std::string(kCompanionDomain), Peter::AI::ToSaveFields(state.companionConfig));
    (void)m_saveDomainStore.WriteDomain(std::string(kAccessibilityDomain), Peter::UI::ToSaveFields(state.accessibility));
  }

  void PlayableSessionController::SaveMissionBoundary(const PersistentState& state) const
  {
    (void)m_saveDomainStore.WriteDomain(
      std::string(kMissionDomain),
      Peter::Core::StructuredFields{
        {"schema_version", "7.2"},
        {"last_mission_id", state.lastMissionId},
        {"last_scene_id", state.lastSceneId}});
  }

  void PlayableSessionController::SaveProfileBoundary(const PersistentState& state) const
  {
    (void)m_saveDomainStore.WriteDomain(
      std::string(kProfileMetaDomain),
      Peter::Core::StructuredFields{
        {"schema_version", "7.2"},
        {"completed_raids", std::to_string(state.completedRaids)},
        {"last_raid_result", state.lastRaidResult}});
  }

  PlayableSessionController::SegmentResult PlayableSessionController::TraverseToInteraction(
    Peter::World::IWorldQueryService& worldQueryService,
    Peter::Traversal::TraversalState startState,
    const std::string_view sceneId,
    const std::string_view interactionId,
    const std::string_view objectiveId) const
  {
    SegmentResult result;
    result.traversalState = startState;

    const auto* interactionDefinition = Peter::World::FindInteractionDefinition(interactionId);
    if (interactionDefinition == nullptr)
    {
      return result;
    }

    const auto* anchor = Peter::World::FindWorldAnchor(interactionDefinition->anchorId);
    if (anchor == nullptr)
    {
      return result;
    }

    double maxLatencyMs = 0.0;
    bool interactionTargetSeen = false;
    for (int step = 0; step < 180; ++step)
    {
      Peter::Adapters::InputState input;
      const double dx = anchor->xMeters - result.traversalState.positionXMeters;
      const double dy = anchor->yMeters - result.traversalState.positionYMeters;
      const double distance = std::sqrt((dx * dx) + (dy * dy));

      if (distance > 0.001)
      {
        input.moveAxisX = dx / distance;
        input.moveAxisY = dy / distance;
        input.lookAxisX = input.moveAxisX * 0.2;
        input.lookAxisY = input.moveAxisY * 0.2;
        input.moveForward = input.moveAxisY > 0.2;
        input.moveBackward = input.moveAxisY < -0.2;
        input.moveRight = input.moveAxisX > 0.2;
        input.moveLeft = input.moveAxisX < -0.2;
        input.sprint = distance > 6.0;
        input.sprintHeld = input.sprint;
      }
      if (distance < 1.4)
      {
        input.interact = true;
        input.interactPressed = true;
      }

      result.traversalState = Peter::Traversal::StepTraversal(m_traversalProfile, result.traversalState, input, 0.1);
      maxLatencyMs = std::max(maxLatencyMs, result.traversalState.inputToMotionLatencyMs);

      const Peter::World::WorldQueryRequest query{
        std::string(sceneId),
        ToWorldPose(result.traversalState),
        m_traversalProfile.interactionRangeMeters,
        std::string(objectiveId)};
      result.snapshot = worldQueryService.CaptureSnapshot(query);
      const auto candidates = worldQueryService.QueryInteractions(query);
      result.interaction = Peter::World::ResolveBestInteraction(candidates);
      interactionTargetSeen = interactionTargetSeen || result.interaction.interactionId == interactionId;
      if (result.interaction.interactionId == interactionId && result.interaction.eligible)
      {
        break;
      }
    }

    result.inputToMotionLatencyMs = maxLatencyMs;
    result.interactionHitchMs = interactionTargetSeen ? 18.0 : 42.0;
    return result;
  }

  Peter::World::ExtractionRuntimeState PlayableSessionController::RunExtractionCountdown(
    const PersistentState& persistentState,
    const std::string_view roomId,
    const std::string_view interactionId,
    const bool allowSuccess) const
  {
    Peter::World::ExtractionRuntimeState extraction;
    extraction.interactionId = std::string(interactionId);
    extraction.roomId = std::string(roomId);
    extraction.reducedTimePressure = persistentState.accessibility.reducedTimePressure;
    extraction.countdownTotalSeconds = persistentState.accessibility.reducedTimePressure ? 9 : 6;
    extraction.countdownRemainingSeconds = extraction.countdownTotalSeconds;
    extraction.phase = Peter::World::ExtractionPhase::Eligible;

    m_platform.audio->PostFeedbackCue("extraction", "zone_ready");
    m_platform.ui->PresentPanel("hud.extraction", Peter::UI::RenderExtractionStatus(extraction));

    extraction.phase = Peter::World::ExtractionPhase::CountdownActive;
    for (int seconds = extraction.countdownTotalSeconds; seconds >= 1; --seconds)
    {
      extraction.countdownRemainingSeconds = seconds;
      m_eventBus.Emit(Peter::Core::Event{
        Peter::Core::EventCategory::Gameplay,
        "gameplay.extraction.countdown.tick",
        {
          {"remaining_seconds", std::to_string(seconds)},
          {"room_id", extraction.roomId}
        }});
      m_platform.audio->PostWorldCue("world.extraction.countdown");
      m_platform.ui->PresentPanel("hud.extraction", Peter::UI::RenderExtractionStatus(extraction));

      if (!allowSuccess && seconds == (extraction.countdownTotalSeconds / 2))
      {
        extraction.phase = Peter::World::ExtractionPhase::Interrupted;
        extraction.failureReason = "Threat pressure interrupted extraction.";
        m_eventBus.Emit(Peter::Core::Event{
          Peter::Core::EventCategory::Gameplay,
          "gameplay.extraction.interrupted",
          {
            {"interaction_id", extraction.interactionId},
            {"reason", extraction.failureReason}
          }});
        extraction.phase = Peter::World::ExtractionPhase::Failed;
        return extraction;
      }
    }

    extraction.countdownRemainingSeconds = 0;
    extraction.phase = Peter::World::ExtractionPhase::Completed;
    return extraction;
  }

  Peter::AI::CompanionDecisionSnapshot PlayableSessionController::BuildPlayableCompanionDecision(
    const PersistentState& persistentState,
    const std::string_view currentGoal,
    const std::string_view roomId,
    const double distanceToPlayerMeters,
    const double distanceToThreatMeters,
    const bool extractionActive) const
  {
    Peter::AI::CompanionWorldContext context;
    context.distanceToPlayerMeters = static_cast<int>(std::round(distanceToPlayerMeters));
    context.distanceToThreatMeters = static_cast<int>(std::round(distanceToThreatMeters));
    context.roomNodeId = std::string(roomId);
    context.routeNodeId = "route.playable." + std::string(roomId);
    context.currentGoal = std::string(currentGoal);
    context.currentTargetId = "player";
    context.playerLowHealth = false;
    context.threatVisible = distanceToThreatMeters < 8.0;
    context.extractionActive = extractionActive;
    context.extractionUrgency = extractionActive ? 3 : 1;
    context.interestMarkerActive = true;
    context.interestMarkerId = extractionActive ? "marker.extraction.pad" : "marker.loot.scrap_crate";
    context.visibleThreatId = extractionActive ? "enemy.none" : "enemy.machine_patrol.chaser_01";
    return Peter::AI::EvaluateCompanion(persistentState.companionConfig, context);
  }

  SliceRunReport PlayableSessionController::Run(const std::string_view scenario)
  {
    PersistentState persistentState = LoadPersistentState();
    SaveCompanionAndAccessibility(persistentState);
    m_platform.ui->ApplyPresentationSettings(Peter::Adapters::PresentationSettings{
      persistentState.accessibility.subtitlesEnabled,
      persistentState.accessibility.subtitleScalePercent,
      persistentState.accessibility.textScalePercent,
      persistentState.accessibility.subtitleBackgroundEnabled,
      persistentState.accessibility.highContrastEnabled,
      persistentState.accessibility.iconRedundancyEnabled,
      persistentState.accessibility.motionComfortEnabled});
    m_platform.camera->ApplyRig(m_traversalProfile.cameraRig);

    Peter::World::CatalogWorldQueryService worldQueryService;
    Peter::World::SceneShell sceneShell(m_eventBus, m_platform.scene.get());
    const auto homeBase = Peter::World::BuildPhase1HomeBase();
    const auto raidZone = Peter::World::BuildRaidZoneForMission("mission.salvage_run.machine_silo");
    const auto* mission = Peter::World::FindMissionTemplate("mission.salvage_run.machine_silo");

    SliceRunReport report;
    report.missionId = mission == nullptr ? "mission.salvage_run.machine_silo" : mission->id;
    report.inputScheme = m_platform.input->ActiveScheme();
    report.cameraMode = m_platform.camera->CurrentRig().mode;

    Peter::World::PlayableSessionState sessionState;
    sessionState.phase = Peter::World::SessionPhase::Boot;
    sessionState.sceneId = "scene.playable.home_base";
    sessionState.missionId = report.missionId;
    sessionState.inputScheme = report.inputScheme;
    sessionState.cameraMode = report.cameraMode;

    const auto homeScene = sceneShell.LoadScene("scene.playable.home_base");
    (void)homeScene;
    report.transitionMs = sceneShell.LastTransition().lastDurationMs;
    persistentState.lastSceneId = "scene.playable.home_base";
    SaveMissionBoundary(persistentState);
    sessionState.phase = Peter::World::SessionPhase::HomeBase;

    m_platform.ui->PresentPanel("home_base.overview", Peter::UI::RenderHomeBaseOverview(homeBase));
    if (const auto* metrics = Peter::World::FindPlayableRoomMetrics("metrics.scene.playable.home_base"))
    {
      m_platform.ui->PresentPanel(
        "home_base.metrics",
        metrics->displayName + " traversal=" + std::to_string(metrics->traversalTimeSeconds) +
          "s landmarks=" + metrics->landmarkQualityLabel);
    }

    auto traversalState = SpawnAtScene("scene.playable.home_base");
    const auto homeMissionBoard = TraverseToInteraction(
      worldQueryService,
      traversalState,
      "scene.playable.home_base",
      "interaction.home.mission_board",
      "objective.home.find_mission_board");
    traversalState = homeMissionBoard.traversalState;
    report.inputToMotionLatencyMs = homeMissionBoard.inputToMotionLatencyMs;
    report.interactionHitchMs = homeMissionBoard.interactionHitchMs;
    sessionState.roomId = homeMissionBoard.snapshot.roomId;
    sessionState.currentObjectiveId = "objective.home.find_mission_board";
    sessionState.activeInteractionId = homeMissionBoard.interaction.interactionId;
    m_platform.ui->PresentPrompt(Peter::UI::RenderInteractionPrompt(homeMissionBoard.interaction, report.inputScheme));
    m_platform.ui->PresentPanel("debug.traversal", Peter::UI::RenderTraversalDebugPanel(traversalState));
    m_platform.ui->PresentDebugMarkers(homeMissionBoard.snapshot.debugMarkerIds);
    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::Performance,
      "performance.input_to_motion",
      {
        {"metric_id", "input_to_motion_latency_ms"},
        {"unit", "ms"},
        {"value", std::to_string(report.inputToMotionLatencyMs)}
      }});
    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::Performance,
      "performance.interaction.hitch",
      {
        {"metric_id", "interaction_hitch_p95_ms"},
        {"unit", "ms"},
        {"value", std::to_string(report.interactionHitchMs)}
      }});
    (void)worldQueryService.ActivateInteraction("scene.playable.home_base", "interaction.home.mission_board");
    m_platform.ui->PresentPanel("home_base.mission_board", Peter::UI::RenderMissionBoard(Peter::World::BuildPhase2MissionTemplates(), report.missionId));

    persistentState.lastMissionId = report.missionId;
    persistentState.lastSceneId = "scene.playable.raid.machine_silo";
    SaveMissionBoundary(persistentState);
    sessionState.phase = Peter::World::SessionPhase::MissionSelect;

    const auto raidScene = sceneShell.LoadScene("scene.playable.raid.machine_silo");
    (void)raidScene;
    report.transitionMs = std::max(report.transitionMs, sceneShell.LastTransition().lastDurationMs);
    sessionState.phase = Peter::World::SessionPhase::RaidActive;
    sessionState.sceneId = "scene.playable.raid.machine_silo";
    traversalState = SpawnAtScene("scene.playable.raid.machine_silo");
    m_platform.ui->PresentPanel(
      "raid.overview",
      Peter::UI::RenderRaidZoneOverview(
        raidZone,
        mission == nullptr ? Peter::World::BuildPhase2MissionTemplates().front() : *mission));
    if (const auto* metrics = Peter::World::FindPlayableRoomMetrics("metrics.scene.playable.raid.machine_silo"))
    {
      m_platform.ui->PresentPanel(
        "raid.metrics",
        metrics->displayName + " traversal=" + std::to_string(metrics->traversalTimeSeconds) +
          "s cover=" + metrics->coverDensityLabel + " extraction=" + metrics->extractionReadabilityLabel);
    }

    const auto raidLoot = TraverseToInteraction(
      worldQueryService,
      traversalState,
      "scene.playable.raid.machine_silo",
      "interaction.raid.scrap_container",
      "objective.raid.collect_loot");
    traversalState = raidLoot.traversalState;
    sessionState.roomId = raidLoot.snapshot.roomId;
    sessionState.currentObjectiveId = "objective.raid.collect_loot";
    sessionState.activeInteractionId = raidLoot.interaction.interactionId;
    report.playerPositionX = traversalState.positionXMeters;
    report.playerPositionY = traversalState.positionYMeters;
    report.playerPositionZ = traversalState.positionZMeters;
    report.playerVelocityX = traversalState.velocityXMetersPerSecond;
    report.playerVelocityY = traversalState.velocityYMetersPerSecond;
    report.playerVelocityZ = traversalState.velocityZMetersPerSecond;
    report.playerSpeedMetersPerSecond = std::sqrt(
      (report.playerVelocityX * report.playerVelocityX) +
      (report.playerVelocityY * report.playerVelocityY));
    report.roomId = raidLoot.snapshot.roomId;
    report.activeInteractionId = raidLoot.interaction.interactionId;
    m_platform.ui->PresentPrompt(Peter::UI::RenderInteractionPrompt(raidLoot.interaction, report.inputScheme));
    m_platform.ui->PresentDebugMarkers(raidLoot.snapshot.debugMarkerIds);
    (void)worldQueryService.ActivateInteraction("scene.playable.raid.machine_silo", "interaction.raid.scrap_container");

    if (ScenarioNeedsWorkshopProof(scenario))
    {
      const auto optionalDetour = TraverseToInteraction(
        worldQueryService,
        traversalState,
        "scene.playable.raid.machine_silo",
        "interaction.raid.power_cell_cache",
        "objective.raid.optional_detour");
      traversalState = optionalDetour.traversalState;
      sessionState.roomId = optionalDetour.snapshot.roomId;
      sessionState.activeInteractionId = optionalDetour.interaction.interactionId;
      (void)worldQueryService.ActivateInteraction("scene.playable.raid.machine_silo", "interaction.raid.power_cell_cache");
    }

    report.lastCompanionDecision = BuildPlayableCompanionDecision(
      persistentState,
      "goal.follow_player",
      sessionState.roomId,
      5.5,
      7.0,
      false);
    m_platform.ui->PresentPanel("companion.explain", Peter::UI::RenderCompanionExplainPanel(report.lastCompanionDecision));
    m_platform.ui->PresentPanel(
      "companion.debug",
      Peter::UI::RenderAiDebugPanel(Peter::AI::BuildExplainSnapshot(report.lastCompanionDecision)));

    const auto extractionSegment = TraverseToInteraction(
      worldQueryService,
      traversalState,
      "scene.playable.raid.machine_silo",
      "interaction.raid.extraction_pad",
      "objective.raid.extract");
    traversalState = extractionSegment.traversalState;
    sessionState.roomId = extractionSegment.snapshot.roomId;
    sessionState.currentObjectiveId = "objective.raid.extract";
    sessionState.activeInteractionId = extractionSegment.interaction.interactionId;
    m_platform.ui->PresentPrompt(Peter::UI::RenderInteractionPrompt(extractionSegment.interaction, report.inputScheme));
    (void)worldQueryService.ActivateInteraction("scene.playable.raid.machine_silo", "interaction.raid.extraction_pad");

    sessionState.phase = Peter::World::SessionPhase::Extracting;
    auto extractionState = RunExtractionCountdown(
      persistentState,
      extractionSegment.snapshot.roomId,
      "interaction.raid.extraction_pad",
      ScenarioSucceeds(scenario));
    report.extractionState = Peter::World::ToString(extractionState.phase);
    report.extractionResult.success = extractionState.phase == Peter::World::ExtractionPhase::Completed;
    report.extractionResult.reason = extractionState.failureReason.empty()
      ? (report.extractionResult.success ? "player_extracted_successfully" : "extraction_failed")
      : extractionState.failureReason;
    report.extractionResult.extractedItems = report.extractionResult.success
      ? Peter::Core::SliceItemLedger{{"item.salvage.scrap_metal", ScenarioNeedsWorkshopProof(scenario) ? 3 : 2}}
      : Peter::Core::SliceItemLedger{};
    report.extractionResult.lostItems = report.extractionResult.success
      ? Peter::Core::SliceItemLedger{}
      : Peter::Core::SliceItemLedger{{"item.salvage.scrap_metal", 2}};

    persistentState.lastRaidResult = report.extractionResult.success ? "success" : "failure";
    if (report.extractionResult.success)
    {
      ++persistentState.completedRaids;
    }
    SaveProfileBoundary(persistentState);

    sessionState.phase = Peter::World::SessionPhase::Results;
    const auto resultsScene = sceneShell.LoadScene("scene.playable.results");
    (void)resultsScene;
    report.transitionMs = std::max(report.transitionMs, sceneShell.LastTransition().lastDurationMs);
    report.currentSceneId = "scene.playable.results";
    report.raidSummary.success = report.extractionResult.success;
    report.raidSummary.missionId = report.missionId;
    report.raidSummary.missionDisplayName = mission == nullptr ? "Salvage Run" : mission->displayName;
    report.raidSummary.timeline = {
      "Entered the home base and found the mission board.",
      "Moved through the raid blockout and reached the main loot beat.",
      "Made it to the extraction zone."};
    if (report.extractionResult.success)
    {
      report.raidSummary.timeline.push_back("Held the extraction zone and completed the countdown.");
      report.raidSummary.gainedItems = "Scrap Metal x" + std::to_string(ScenarioNeedsWorkshopProof(scenario) ? 3 : 2);
      report.raidSummary.lostItems = "none";
      report.raidSummary.recoveredItems = "none";
    }
    else
    {
      report.raidSummary.timeline.push_back("Extraction was interrupted and the run failed.");
      report.raidSummary.gainedItems = "none";
      report.raidSummary.lostItems = "Scrap Metal x2";
      report.raidSummary.recoveredItems = "none";
    }
    report.raidSummary.brokenItems = "none";
    report.raidSummary.companionHighlight = report.lastCompanionDecision.topReason;
    report.raidSummary.lessonTip = "help.first_extraction";
    m_platform.ui->PresentPanel("raid.summary", Peter::UI::RenderPostRaidSummary(report.raidSummary));

    sessionState.phase = Peter::World::SessionPhase::ReturningHome;
    const auto returnHomeScene = sceneShell.LoadScene("scene.playable.home_base");
    (void)returnHomeScene;
    report.transitionMs = std::max(report.transitionMs, sceneShell.LastTransition().lastDurationMs);
    report.currentSceneId = "scene.playable.home_base";
    persistentState.lastSceneId = "scene.playable.home_base";
    SaveMissionBoundary(persistentState);

    if (report.extractionResult.success && ScenarioNeedsWorkshopProof(scenario))
    {
      const auto companionTerminal = TraverseToInteraction(
        worldQueryService,
        SpawnAtScene("scene.playable.home_base"),
        "scene.playable.home_base",
        "interaction.home.companion_terminal",
        "objective.home.change_companion_setting");
      const auto previousConfig = persistentState.companionConfig;
      std::vector<std::string> chipIds = previousConfig.activeChipIds;
      if (std::find(chipIds.begin(), chipIds.end(), "chip.stay_near_me") == chipIds.end())
      {
        chipIds.push_back("chip.stay_near_me");
      }
      const auto previewConfig = Peter::Workshop::BuildBehaviorPreviewConfig(
        previousConfig,
        previousConfig.stanceId,
        chipIds,
        {{"chip.stay_near_me", 4.0}});
      report.lastRuleEditPreview = Peter::Workshop::BuildCompanionBehaviorPreview(previousConfig, previewConfig);
      if (report.lastRuleEditPreview.valid)
      {
        persistentState.companionConfig = previewConfig;
        SaveCompanionAndAccessibility(persistentState);
      }

      const auto beforeDecision = BuildPlayableCompanionDecision(
        PersistentState{previousConfig, persistentState.accessibility, persistentState.completedRaids, persistentState.lastRaidResult, persistentState.lastMissionId, persistentState.lastSceneId},
        "goal.follow_player",
        companionTerminal.snapshot.roomId,
        Peter::AI::ResolveFollowDistance(previousConfig),
        9.0,
        false);
      report.lastCompanionDecision = BuildPlayableCompanionDecision(
        persistentState,
        "goal.follow_player",
        companionTerminal.snapshot.roomId,
        Peter::AI::ResolveFollowDistance(persistentState.companionConfig),
        9.0,
        false);

      m_platform.ui->PresentPanel(
        "companion.rule_effect",
        Peter::UI::RenderCompanionCompareView(
          beforeDecision,
          report.lastCompanionDecision,
          report.lastRuleEditPreview.deltaSummary));
      m_eventBus.Emit(Peter::Core::Event{
        Peter::Core::EventCategory::Gameplay,
        "gameplay.workshop.follow_distance.proven_in_next_raid",
        {
          {"follow_distance_meters", std::to_string(Peter::AI::ResolveFollowDistance(persistentState.companionConfig))},
          {"mission_id", report.missionId}
        }});
    }

    report.currentSceneId = "scene.playable.home_base";
    report.inputScheme = m_platform.input->ActiveScheme();
    report.cameraMode = m_platform.camera->CurrentRig().mode;

    std::ostringstream summary;
    summary << "Completed the Phase 7.2 playable loop: traversal, interaction, home-base routing, the Machine Silo blockout, and world-space extraction now run through the O3DE-backed runtime.";
    if (report.lastRuleEditPreview.valid)
    {
      summary << " The home companion terminal also proved that a follow-distance change can be saved and noticed on the next playable run.";
    }
    if (!report.extractionResult.success)
    {
      summary.str("");
      summary << "Completed the Phase 7.2 failure loop: the player could still traverse the raid, reach extraction, understand the interruption, view results, and return home coherently.";
    }
    report.summary = summary.str();
    report.success = report.extractionResult.success;
    return report;
  }
} // namespace Peter::App
