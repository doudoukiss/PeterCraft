#include "Phase1Slice.h"

#include "PeterCombat/EncounterSimulator.h"
#include "PeterTools/ScenarioHarness.h"
#include "PeterValidation/ValidationModule.h"
#include "PeterWorld/SceneShell.h"

#include <algorithm>
#include <chrono>
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
    constexpr std::string_view kCreatorManifestDomain = "save_domain.creator_manifest";
    constexpr std::string_view kCreatorProgressDomain = "save_domain.creator_progress";
    constexpr std::string_view kCreatorSettingsDomain = "save_domain.creator_settings";

    template <typename Fn>
    auto MeasureMilliseconds(Fn&& function, double& durationMs) -> decltype(function())
    {
      const auto started = std::chrono::steady_clock::now();
      auto result = function();
      const auto finished = std::chrono::steady_clock::now();
      durationMs = std::chrono::duration<double, std::milli>(finished - started).count();
      return result;
    }

    void EmitPerformanceMetric(
      Peter::Core::EventBus& eventBus,
      std::string_view name,
      std::string_view metricId,
      const double value,
      std::string_view unit,
      Peter::Core::StructuredFields extra = {})
    {
      extra["metric_id"] = std::string(metricId);
      extra["unit"] = std::string(unit);
      extra["value"] = std::to_string(value);
      eventBus.Emit(Peter::Core::Event{
        Peter::Core::EventCategory::Performance,
        std::string(name),
        std::move(extra)});
    }

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

    Peter::AI::CompanionWorldContext BuildExplainPreviewContext(const Peter::Progression::WorkshopState& workshopState)
    {
      Peter::AI::CompanionWorldContext context;
      context.rareLootVisible = false;
      context.lootPingUnlocked = false;
      context.guardProtocolUnlocked = HasNode(workshopState, "track.companion_capabilities.guard_protocol");
      context.repairPulseUnlocked = HasNode(workshopState, "track.companion_capabilities.repair_pulse");
      context.distanceToPlayerMeters = 7;
      context.distanceToThreatMeters = 6;
      context.roomNodeId = "room.raid.patrol_hall";
      context.routeNodeId = "route.machine_silo.entry_loop";
      context.currentGoal = "goal.follow_player";
      context.currentTargetId = "player";
      return context;
    }

    Peter::Workshop::TinyScriptHookKind HookKindFromString(const std::string_view hookId)
    {
      if (hookId == "mission.score_bonus")
      {
        return Peter::Workshop::TinyScriptHookKind::MissionScoreBonus;
      }
      if (hookId == "tutorial.message_override")
      {
        return Peter::Workshop::TinyScriptHookKind::TutorialMessageOverride;
      }
      if (hookId == "loot.rarity_reaction")
      {
        return Peter::Workshop::TinyScriptHookKind::LootRarityReaction;
      }
      return Peter::Workshop::TinyScriptHookKind::CompanionPriorityHint;
    }

    std::map<std::string, double, std::less<>> LoadActiveTinkerValues(
      const Peter::Core::CreatorContentStore& store,
      const Peter::Workshop::CreatorManifest& manifest)
    {
      auto values = Peter::Workshop::DefaultTinkerValues();
      const auto activeId = manifest.activeDraftIds.find("tinker");
      if (activeId == manifest.activeDraftIds.end())
      {
        return values;
      }

      const auto fields = store.ReadArtifact(Peter::Core::CreatorContentKind::TinkerPreset, activeId->second);
      for (const auto& variable : Peter::Workshop::BuildPhase4TinkerVariables())
      {
        const auto iterator = fields.find(variable.id);
        if (iterator != fields.end())
        {
          values[variable.id] = std::stod(iterator->second);
        }
      }
      return values;
    }

    const Peter::Workshop::LogicRulesetDefinition* LoadActiveLogicRuleset(
      const Peter::Core::CreatorContentStore& store,
      const Peter::Workshop::CreatorManifest& manifest)
    {
      const auto activeId = manifest.activeDraftIds.find("logic");
      if (activeId == manifest.activeDraftIds.end())
      {
        return nullptr;
      }

      const auto fields = store.ReadArtifact(Peter::Core::CreatorContentKind::LogicRules, activeId->second);
      const auto templateId = fields.find("logic_template_id");
      if (templateId != fields.end())
      {
        return Peter::Workshop::FindLogicTemplate(templateId->second);
      }
      return Peter::Workshop::FindLogicTemplate(activeId->second);
    }

    Peter::Workshop::TinyScriptDefinition LoadActiveTinyScript(
      const Peter::Core::CreatorContentStore& store,
      const Peter::Workshop::CreatorManifest& manifest)
    {
      const auto activeId = manifest.activeDraftIds.find("script");
      if (activeId == manifest.activeDraftIds.end())
      {
        return {};
      }

      const auto fields = store.ReadArtifact(Peter::Core::CreatorContentKind::TinyScript, activeId->second);
      Peter::Workshop::TinyScriptDefinition script;
      script.id = activeId->second;
      script.displayName = fields.contains("display_name") ? fields.at("display_name") : activeId->second;
      script.summary = fields.contains("summary") ? fields.at("summary") : "Creator-authored tiny script.";
      script.body = fields.contains("body") ? fields.at("body") : "";
      script.targetActionId = fields.contains("target_action_id") ? fields.at("target_action_id") : "";
      script.hookKind = fields.contains("hook_kind")
        ? HookKindFromString(fields.at("hook_kind"))
        : Peter::Workshop::TinyScriptHookKind::CompanionPriorityHint;
      if (script.body.empty())
      {
        if (const auto* templateScript = Peter::Workshop::FindTinyScriptTemplate(activeId->second))
        {
          return *templateScript;
        }
      }
      return script;
    }

    Peter::Workshop::MiniMissionDraftDefinition LoadActiveMiniMission(
      const Peter::Core::CreatorContentStore& store,
      const Peter::Workshop::CreatorManifest& manifest)
    {
      const auto activeId = manifest.activeDraftIds.find("mini_mission");
      if (activeId == manifest.activeDraftIds.end())
      {
        return {};
      }

      const auto fields = store.ReadArtifact(Peter::Core::CreatorContentKind::MiniMission, activeId->second);
      Peter::Workshop::MiniMissionDraftDefinition draft;
      draft.id = activeId->second;
      draft.displayName = fields.contains("display_name") ? fields.at("display_name") : activeId->second;
      draft.summary = fields.contains("summary") ? fields.at("summary") : "Creator-authored mini mission.";
      draft.roomBundleId = fields.contains("room_bundle_id") ? fields.at("room_bundle_id") : "";
      draft.lootGoalItemId = fields.contains("loot_goal_item_id") ? fields.at("loot_goal_item_id") : "";
      draft.enemyGroupId = fields.contains("enemy_group_id") ? fields.at("enemy_group_id") : "";
      draft.extractionPointId = fields.contains("extraction_point_id") ? fields.at("extraction_point_id") : "";
      draft.rewardBundleId = fields.contains("reward_bundle_id") ? fields.at("reward_bundle_id") : "";
      draft.active = true;
      draft.revision = fields.contains("revision") ? std::stoi(fields.at("revision")) : 1;
      return draft;
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
    , m_creatorContentStore(m_profile, m_eventBus)
    , m_qualityProfile(Peter::Core::LoadPhase6QualityProfile())
    , m_homeBase(Peter::World::BuildPhase1HomeBase())
    , m_raidZone(Peter::World::BuildPhase1RaidZone())
    , m_traversal(Peter::Traversal::BuildTraversalProfile(m_qualityProfile))
  {
  }

  Phase1Slice::PersistentState Phase1Slice::LoadPersistentState() const
  {
    const auto started = std::chrono::steady_clock::now();
    PersistentState state;
    m_creatorContentStore.EnsureLayout();
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
      const auto fields = m_saveDomainStore.ReadDomain(std::string(kCompanionDomain));
      const auto schemaVersion = fields.find("schema_version");
      if (schemaVersion == fields.end() || schemaVersion->second == "2")
      {
        m_eventBus.Emit(Peter::Core::Event{
          Peter::Core::EventCategory::SaveLoad,
          "save_load.migration.phase2_to_phase3.companion_config",
          {{"domain_id", std::string(kCompanionDomain)}, {"profile_id", m_profile.profileId}}});
      }
      state.companionConfig = Peter::AI::CompanionConfigFromSaveFields(fields);
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
      const auto lastScene = fields.find("last_scene_id");
      state.lastMissionId = lastMission == fields.end() ? state.lastMissionId : lastMission->second;
      state.lastSceneId = lastScene == fields.end() ? state.lastSceneId : lastScene->second;
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

    if (m_saveDomainStore.DomainExists(kCreatorManifestDomain))
    {
      state.creatorManifest = Peter::Workshop::CreatorManifestFromSaveFields(
        m_saveDomainStore.ReadDomain(std::string(kCreatorManifestDomain)));
    }

    if (m_saveDomainStore.DomainExists(kCreatorProgressDomain))
    {
      state.creatorProgress = Peter::Workshop::CreatorProgressStateFromSaveFields(
        m_saveDomainStore.ReadDomain(std::string(kCreatorProgressDomain)));
    }

    if (m_saveDomainStore.DomainExists(kCreatorSettingsDomain))
    {
      state.creatorSettings = Peter::Workshop::CreatorSettingsFromSaveFields(
        m_saveDomainStore.ReadDomain(std::string(kCreatorSettingsDomain)));
    }

    const auto invalidateCreatorArtifact = [&](const std::string_view kindId, const std::string_view contentId) {
      state.creatorManifest.disabledContent[std::string(kindId)] = true;
      state.creatorManifest.activeDraftIds.erase(std::string(kindId));
      m_eventBus.Emit(Peter::Core::Event{
        Peter::Core::EventCategory::CreatorTools,
        "creator_tools.content.auto_disabled",
        {
          {"content_id", std::string(contentId)},
          {"kind", std::string(kindId)},
          {"reason", "invalid_or_unreadable_artifact"}
        }});
    };

    std::vector<std::pair<std::string, std::string>> activeArtifacts;
    for (const auto& [kindId, contentId] : state.creatorManifest.activeDraftIds)
    {
      activeArtifacts.emplace_back(kindId, contentId);
    }

    for (const auto& [kindId, contentId] : activeArtifacts)
    {
      if (kindId == "logic")
      {
        const auto readResult =
          m_creatorContentStore.ReadArtifactChecked(Peter::Core::CreatorContentKind::LogicRules, contentId);
        if (!readResult.valid)
        {
          invalidateCreatorArtifact(kindId, contentId);
        }
      }
      else if (kindId == "script")
      {
        const auto readResult =
          m_creatorContentStore.ReadArtifactChecked(Peter::Core::CreatorContentKind::TinyScript, contentId);
        if (!readResult.valid)
        {
          invalidateCreatorArtifact(kindId, contentId);
        }
      }
      else if (kindId == "tinker")
      {
        const auto readResult =
          m_creatorContentStore.ReadArtifactChecked(Peter::Core::CreatorContentKind::TinkerPreset, contentId);
        if (!readResult.valid)
        {
          invalidateCreatorArtifact(kindId, contentId);
        }
      }
      else if (kindId == "mini_mission")
      {
        const auto readResult =
          m_creatorContentStore.ReadArtifactChecked(Peter::Core::CreatorContentKind::MiniMission, contentId);
        if (!readResult.valid)
        {
          invalidateCreatorArtifact(kindId, contentId);
        }
      }
    }

    state.tinkerValues = LoadActiveTinkerValues(m_creatorContentStore, state.creatorManifest);
    state.companionConfig = Peter::Workshop::ApplyTinkerValues(state.companionConfig, state.tinkerValues, false);

    const auto favoriteValidation =
      Peter::Validation::ValidateFavoriteItemReference(state.loadout.favoriteItemId, state.inventory);
    if (!favoriteValidation.valid || state.loadout.favoriteItemId.empty())
    {
      state.loadout.favoriteItemId = state.loadout.equippedToolId;
    }

    const auto accessibilityValidation =
      Peter::Validation::ValidateAccessibilitySettings(state.accessibility, m_qualityProfile);
    if (!accessibilityValidation.valid)
    {
      state.accessibility = Peter::UI::AccessibilitySettingsFromSaveFields({}, defaultBindings);
      m_eventBus.Emit(Peter::Core::Event{
        Peter::Core::EventCategory::Validation,
        "validation.accessibility.reset_to_safe_defaults",
        {{"reason", accessibilityValidation.message}}});
    }

    const auto finished = std::chrono::steady_clock::now();
    EmitPerformanceMetric(
      m_eventBus,
      "performance.save.full_load",
      "full_load_ms",
      std::chrono::duration<double, std::milli>(finished - started).count(),
      "ms",
      {{"profile_id", m_profile.profileId}});

    return state;
  }

  void Phase1Slice::SavePersistentState(const PersistentState& state) const
  {
    double totalSaveDurationMs = 0.0;
    totalSaveDurationMs += m_saveDomainStore.WriteDomain(
      std::string(kInventoryDomain),
      Peter::Inventory::ToSaveFields(state.inventory, state.loadout)).durationMs;
    totalSaveDurationMs += m_saveDomainStore.WriteDomain(
      std::string(kRecoveryDomain),
      Peter::Inventory::RecoveryStateToSaveFields(state.recovery)).durationMs;
    totalSaveDurationMs += m_saveDomainStore.WriteDomain(
      std::string(kWorkshopDomain),
      Peter::Progression::ToSaveFields(state.workshop)).durationMs;
    totalSaveDurationMs += m_saveDomainStore.WriteDomain(
      std::string(kCompanionDomain),
      Peter::AI::ToSaveFields(state.companionConfig)).durationMs;
    totalSaveDurationMs += m_saveDomainStore.WriteDomain(
      std::string(kAccessibilityDomain),
      Peter::UI::ToSaveFields(state.accessibility)).durationMs;
    totalSaveDurationMs += m_saveDomainStore.WriteDomain(
      std::string(kProfileMetaDomain),
      Peter::Core::StructuredFields{
        {"schema_version", "2"},
        {"completed_raids", std::to_string(state.completedRaids)},
        {"last_raid_result", state.lastRaidResult},
        {"profile_id", m_profile.profileId}
      }).durationMs;
    totalSaveDurationMs += m_saveDomainStore.WriteDomain(
      std::string(kTutorialDomain),
      Peter::Core::StructuredFields{
        {"schema_version", "2"},
        {"completed_lessons", JoinStrings(state.completedLessons)},
        {"guided_first_run_complete", state.guidedFirstRunComplete ? "true" : "false"},
        {"rule_edit_complete", state.ruleEditComplete ? "true" : "false"},
        {"tutorial_hint_level", std::to_string(state.tutorialHintLevel)}
      }).durationMs;
    totalSaveDurationMs += m_saveDomainStore.WriteDomain(
      std::string(kMissionDomain),
      Peter::Core::StructuredFields{
        {"schema_version", "2"},
        {"last_mission_id", state.lastMissionId},
        {"last_scene_id", state.lastSceneId},
        {"last_result", state.lastRaidResult}
      }).durationMs;
    totalSaveDurationMs += m_saveDomainStore.WriteDomain(
      std::string(kCreatorManifestDomain),
      Peter::Workshop::ToSaveFields(state.creatorManifest)).durationMs;
    totalSaveDurationMs += m_saveDomainStore.WriteDomain(
      std::string(kCreatorProgressDomain),
      Peter::Workshop::ToSaveFields(state.creatorProgress)).durationMs;
    totalSaveDurationMs += m_saveDomainStore.WriteDomain(
      std::string(kCreatorSettingsDomain),
      Peter::Workshop::ToSaveFields(state.creatorSettings)).durationMs;

    EmitPerformanceMetric(
      m_eventBus,
      "performance.save.full_save",
      "full_save_ms",
      totalSaveDurationMs,
      "ms",
      {{"profile_id", m_profile.profileId}});
  }

  void Phase1Slice::PresentHomeBase(const PersistentState& state) const
  {
    auto cameraRig = m_traversal.cameraRig;
    cameraRig.followDistanceMeters =
      std::max(
        std::max(cameraRig.followDistanceMeters, Peter::AI::ResolveFollowDistance(state.companionConfig) - 1.0),
        m_qualityProfile.movement.companionSpacingMeters);
    m_platform.camera->ApplyRig(cameraRig);
    m_platform.ui->ApplyPresentationSettings(Peter::Adapters::PresentationSettings{
      state.accessibility.subtitlesEnabled,
      state.accessibility.subtitleScalePercent,
      state.accessibility.textScalePercent,
      state.accessibility.subtitleBackgroundEnabled,
      state.accessibility.highContrastEnabled,
      state.accessibility.iconRedundancyEnabled,
      state.accessibility.motionComfortEnabled});
    m_platform.ui->PresentState("ui.home_base");
    m_platform.ui->PresentPanel("home_base.overview", Peter::UI::RenderHomeBaseOverview(m_homeBase));
    m_platform.ui->PresentPanel("home_base.companion_editor", Peter::UI::RenderCompanionBehaviorEditor(state.companionConfig));

    double accessibilityRenderMs = 0.0;
    const auto accessibilityPanel = MeasureMilliseconds(
      [&]() {
        return Peter::UI::RenderAccessibilitySettings(state.accessibility, m_platform.input->DefaultBindings());
      },
      accessibilityRenderMs);
    EmitPerformanceMetric(
      m_eventBus,
      "performance.ui.panel_render",
      "ui_render_p95_ms",
      accessibilityRenderMs,
      "ms",
      {{"panel_id", "home_base.accessibility"}});
    m_platform.ui->PresentPanel("home_base.accessibility", accessibilityPanel);

    m_platform.ui->PresentPanel(
      "home_base.onboarding",
      Peter::UI::RenderOnboardingFunnel({
        {"onboarding.first_mission", "onboarding.first_mission", "help.first_mission", state.lastRaidResult != "none"},
        {"onboarding.first_extraction", "onboarding.first_extraction", "help.first_extraction", state.completedRaids > 0},
        {"onboarding.first_repair", "onboarding.first_repair", "help.first_repair", HasLesson(state.completedLessons, "lesson.phase2.first_repair")},
        {"onboarding.first_creator_change", "onboarding.first_creator_change", "help.first_creator_change", state.ruleEditComplete},
        {"onboarding.first_explain_panel", "onboarding.first_explain_panel", "help.first_explain_panel", HasLesson(state.completedLessons, "lesson.phase4.read_explanation")}
      }));
    const auto* activeRuleset = LoadActiveLogicRuleset(m_creatorContentStore, state.creatorManifest);
    const auto activeScript = LoadActiveTinyScript(m_creatorContentStore, state.creatorManifest);
    m_platform.ui->PresentCreatorPanel(
      "home_base.creator_panel",
      Peter::UI::RenderCreatorPanel(
        state.tinkerValues,
        activeRuleset,
        activeScript.id.empty() ? nullptr : &activeScript));

    if (state.creatorProgress.mentorViewUnlocked || Peter::Progression::HasUnlockedNode(
      state.workshop,
      "track.creator_unlocks.mentor_view"))
    {
      const auto previewContext = BuildExplainPreviewContext(state.workshop);
      const auto previewDecision = Peter::AI::EvaluateCompanion(state.companionConfig, previewContext);
      const auto mentorPanel = Peter::UI::RenderMentorSummary(
        state.creatorManifest,
        state.creatorProgress,
        Peter::AI::BuildExplainSnapshot(previewDecision));
      m_platform.ui->PresentPanel("home_base.mentor_summary", mentorPanel);
    }
    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::Gameplay,
      "gameplay.traversal.ready",
      {
        {"camera_feel", Peter::Traversal::DescribeCameraFeel(m_qualityProfile.camera)},
        {"camera_mode", cameraRig.mode},
        {"input_scheme", m_platform.input->ActiveScheme()},
        {"interaction_range_meters", std::to_string(m_traversal.interactionRangeMeters)},
        {"movement_responsiveness", std::to_string(Peter::Traversal::EvaluateMovementResponsivenessScore(m_traversal))},
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
    m_platform.ui->PresentCompanionFeedback(snapshot.calloutToken, snapshot.gestureToken);
    m_platform.ui->PresentDebugMarkers(
      {
        std::string(roomId),
        snapshot.blackboard.routeNodeId,
        snapshot.perception.noticedReason
      });
    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::AI,
      "ai.perception.notice",
      {
        {"alert_level", snapshot.blackboard.alertLevel},
        {"reason", snapshot.perception.noticedReason},
        {"room_id", std::string(roomId)}
      }});
    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::AI,
      "ai.memory.updated",
      {
        {"last_known_threat", snapshot.blackboard.lastKnownThreatPositionToken},
        {"room_id", std::string(roomId)},
        {"target_id", snapshot.blackboard.currentTargetId}
      }});
    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::AI,
      "ai.decision.selected",
      {
        {"action", snapshot.lastAction},
        {"confidence", snapshot.confidenceLabel},
        {"goal", snapshot.currentGoal},
        {"reason", snapshot.topReason},
        {"room_id", std::string(roomId)},
        {"state", snapshot.currentState}
      }});
    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::AI,
      "ai.action.started",
      {
        {"action", snapshot.lastAction},
        {"room_id", std::string(roomId)},
        {"stage", std::string(Peter::AI::ToString(Peter::AI::ActionExecutionStage::Started))}
      }});
    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::AI,
      "ai.companion.action",
      {
        {"action", snapshot.lastAction},
        {"reason", snapshot.topReason},
        {"room_id", std::string(roomId)},
        {"state", snapshot.currentState}
      }});
    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::AI,
      "ai.action.succeeded",
      {
        {"action", snapshot.lastAction},
        {"room_id", std::string(roomId)},
        {"stage", std::string(Peter::AI::ToString(snapshot.lastResult.stage))}
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

      if (step.action == "open_creator_panel")
      {
        const auto* activeRuleset = LoadActiveLogicRuleset(m_creatorContentStore, state.creatorManifest);
        const auto activeScript = LoadActiveTinyScript(m_creatorContentStore, state.creatorManifest);
        m_platform.ui->PresentCreatorPanel(
          "creator.lesson.panel",
          Peter::UI::RenderCreatorPanel(
            state.tinkerValues,
            activeRuleset,
            activeScript.id.empty() ? nullptr : &activeScript));
      }
      else if (step.action == "apply_tinker_preset")
      {
        if (const auto* preset = Peter::Workshop::FindTinkerPreset(step.targetId); preset != nullptr)
        {
          for (const auto& [variableId, value] : preset->values)
          {
            state.tinkerValues[variableId] = value;
          }
          state.companionConfig = Peter::Workshop::ApplyTinkerValues(state.companionConfig, state.tinkerValues, false);
          Peter::Core::StructuredFields fields;
          fields["display_name"] = preset->displayName;
          fields["summary"] = preset->summary;
          for (const auto& [variableId, value] : state.tinkerValues)
          {
            fields[variableId] = std::to_string(value);
          }
          const int revision = m_creatorContentStore.WriteArtifact(
            Peter::Core::CreatorContentKind::TinkerPreset,
            preset->id,
            fields);
          const auto activation = Peter::Workshop::ActivateCreatorArtifact(
            state.creatorManifest,
            "tinker",
            preset->id,
            revision,
            true);
          m_eventBus.Emit(Peter::Core::Event{
            Peter::Core::EventCategory::CreatorTools,
            "creator_tools.tinker.applied",
            {{"preset_id", preset->id}, {"success", activation.success ? "true" : "false"}}});
        }
      }
      else if (step.action == "apply_logic_template")
      {
        if (const auto* ruleset = Peter::Workshop::FindLogicTemplate(step.targetId); ruleset != nullptr)
        {
          const int revision = m_creatorContentStore.WriteArtifact(
            Peter::Core::CreatorContentKind::LogicRules,
            ruleset->id,
            Peter::Core::StructuredFields{
              {"display_name", ruleset->displayName},
              {"logic_template_id", ruleset->id},
              {"summary", ruleset->summary}});
          const auto validation = Peter::Validation::ValidateLogicRulesetDefinition(*ruleset);
          const auto activation = Peter::Workshop::ActivateCreatorArtifact(
            state.creatorManifest,
            "logic",
            ruleset->id,
            revision,
            validation.valid);
          m_platform.ui->PresentCreatorPanel(
            "creator.lesson.logic_validation",
            Peter::UI::RenderCreatorValidationMessage(validation.valid, validation.message));
          m_platform.audio->PostPrioritizedFeedbackCue(
            validation.valid ? "creator_success" : "creator_error",
            validation.valid ? "logic_template" : "logic_template_invalid",
            validation.valid ? 2 : 3,
            !validation.valid);
          m_eventBus.Emit(Peter::Core::Event{
            Peter::Core::EventCategory::CreatorTools,
            "creator_tools.logic.applied",
            {{"ruleset_id", ruleset->id}, {"success", activation.success ? "true" : "false"}}});
        }
      }
      else if (step.action == "edit_script" || step.action == "run_script_validation")
      {
        const auto* templateScript = Peter::Workshop::FindTinyScriptTemplate(step.targetId);
        if (templateScript != nullptr)
        {
          m_platform.ui->PresentTextEditor(
            "creator.lesson.script_editor",
            Peter::UI::RenderTinyScriptEditor(*templateScript));
          const auto validation = Peter::Workshop::ValidateTinyScript(*templateScript);
          const int revision = m_creatorContentStore.WriteArtifact(
            Peter::Core::CreatorContentKind::TinyScript,
            templateScript->id,
            Peter::Core::StructuredFields{
              {"body", templateScript->body},
              {"display_name", templateScript->displayName},
              {"hook_kind", std::string(Peter::Workshop::ToString(templateScript->hookKind))},
              {"summary", templateScript->summary},
              {"target_action_id", templateScript->targetActionId}});
          (void)Peter::Workshop::ActivateCreatorArtifact(
            state.creatorManifest,
            "script",
            templateScript->id,
            revision,
            validation.valid);
          m_platform.ui->PresentCreatorPanel(
            "creator.lesson.script_validation",
            Peter::UI::RenderCreatorValidationMessage(validation.valid, validation.valid ? "Tiny script passed validation." : validation.error));
          m_platform.audio->PostPrioritizedFeedbackCue(
            validation.valid ? "creator_success" : "creator_error",
            validation.valid ? "script_valid" : "script_invalid",
            validation.valid ? 2 : 3,
            !validation.valid);
          m_eventBus.Emit(Peter::Core::Event{
            Peter::Core::EventCategory::CreatorTools,
            "creator_tools.script.validated",
            {{"script_id", templateScript->id}, {"valid", validation.valid ? "true" : "false"}}});
        }
      }
      else if (step.action == "show_replay")
      {
        Peter::Tools::DeterministicScenarioHarness harness(step.targetId.empty() ? "scenario.ai.follow_corridor.v1" : step.targetId);
        const auto compare = harness.Compare(Peter::AI::DefaultCompanionConfig(), state.companionConfig);
        const auto snippet = Peter::Workshop::BuildCreatorReplaySnippet(
          compare.scenarioId,
          compare.beforeActionPath,
          compare.afterActionPath,
          "Your creator change changed the deterministic path.");
        m_platform.ui->PresentReplayTimeline("creator.lesson.replay", Peter::UI::RenderReplaySnippet(snippet));
        m_eventBus.Emit(Peter::Core::Event{
          Peter::Core::EventCategory::CreatorTools,
          "creator_tools.replay.opened",
          {{"changed", compare.changed ? "true" : "false"}, {"scenario_id", compare.scenarioId}}});
      }
      else if (step.action == "launch_creator_mission")
      {
        const auto draft = LoadActiveMiniMission(m_creatorContentStore, state.creatorManifest);
        if (!draft.id.empty())
        {
          const auto mission = Peter::World::BuildMissionFromMiniMissionDraft(draft);
          m_platform.ui->PresentPanel("creator.lesson.mini_mission", Peter::UI::RenderMissionBoard({mission}, mission.id));
        }
        m_eventBus.Emit(Peter::Core::Event{
          Peter::Core::EventCategory::CreatorTools,
          "creator_tools.mini_mission.launched",
          {{"draft_id", step.targetId}}});
      }
      else if (step.action == "reset_creator_content")
      {
        state.tinkerValues = Peter::Workshop::DefaultTinkerValues();
        state.creatorManifest.activeDraftIds.clear();
        state.creatorManifest.disabledContent.clear();
        state.creatorManifest.lastKnownGoodRevisions.clear();
        state.creatorManifest.rollbackTargets.clear();
        state.companionConfig = Peter::Workshop::ApplyTinkerValues(
          Peter::AI::DefaultCompanionConfig(),
          state.tinkerValues,
          false);
        m_eventBus.Emit(Peter::Core::Event{
          Peter::Core::EventCategory::CreatorTools,
          "creator_tools.reset",
          {{"lesson_id", std::string(lessonId)}}});
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
    if (std::string(lessonId).starts_with("lesson.phase4.") &&
      !HasLesson(state.creatorProgress.completedCreatorLessons, lessonId))
    {
      state.creatorProgress.completedCreatorLessons.push_back(std::string(lessonId));
    }
  }

  SliceRunReport Phase1Slice::Run(SliceScenario scenario)
  {
    auto state = LoadPersistentState();
    const auto qualityValidation = Peter::Validation::ValidatePhase6QualityProfile(m_qualityProfile);
    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::Validation,
      "validation.runtime.quality_profile",
      {
        {"message", qualityValidation.message},
        {"profile_id", m_qualityProfile.id},
        {"valid", qualityValidation.valid ? "true" : "false"}
      }});
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
        return RunMissionScenario(state, "mission.recover_artifact.sky_docks", true, false);
      case SliceScenario::EscortSupport:
        return RunMissionScenario(state, "mission.escort_companion.relay_tunnels", true, false);
    }

    return RunMissionScenario(state, "mission.salvage_run.machine_silo", true, false);
  }

  SliceRunReport Phase1Slice::RunMissionScenario(
    PersistentState& state,
    const std::string_view missionId,
    const bool expectSuccess,
    const bool guidedMode)
  {
    auto evaluateCompanion = [&](const Peter::AI::CompanionConfig& config,
                                 const Peter::AI::CompanionWorldContext& context,
                                 const Peter::AI::BehaviorOverrideSet* overrides = nullptr) {
      double decisionMs = 0.0;
      Peter::AI::CompanionDecisionSnapshot snapshot;
      if (overrides == nullptr)
      {
        snapshot = MeasureMilliseconds(
          [&]() { return Peter::AI::EvaluateCompanion(config, context); },
          decisionMs);
      }
      else
      {
        snapshot = MeasureMilliseconds(
          [&]() { return Peter::AI::EvaluateCompanion(config, context, *overrides); },
          decisionMs);
      }
      EmitPerformanceMetric(
        m_eventBus,
        "performance.ai.decision",
        "ai_decision_p95_ms",
        decisionMs,
        "ms",
        {{"goal", context.currentGoal}, {"room_id", context.roomNodeId}});
      return snapshot;
    };

    auto postFeedback = [&](const std::string_view cueFamily,
                            const std::string_view variantId,
                            const int priority,
                            const bool critical,
                            const std::string_view beatId) {
      m_platform.audio->PostPrioritizedFeedbackCue(cueFamily, variantId, priority, critical);
      EmitPerformanceMetric(
        m_eventBus,
        "performance.feedback.world_cue",
        "concurrent_world_feedback",
        1.0,
        "count",
        {{"beat_id", std::string(beatId)}, {"cue_family", std::string(cueFamily)}});
      EmitPerformanceMetric(
        m_eventBus,
        "performance.feedback.critical_beat",
        "critical_feedback_per_beat",
        critical ? 1.0 : 0.0,
        "count",
        {{"beat_id", std::string(beatId)}, {"cue_family", std::string(cueFamily)}});
    };

    Peter::World::SceneShell sceneShell(m_eventBus);
    double transitionMs = 0.0;
    const auto homeScene = MeasureMilliseconds([&]() { return sceneShell.LoadHomeBase(m_homeBase); }, transitionMs);
    EmitPerformanceMetric(
      m_eventBus,
      "performance.scene.transition",
      "transition_ms",
      transitionMs,
      "ms",
      {{"target_scene", m_homeBase.sceneId}});
    (void)homeScene;

    const auto* mission = Peter::World::FindMissionTemplate(missionId);
    if (mission == nullptr)
    {
      throw std::runtime_error("Missing mission template.");
    }
    const auto raidZone = Peter::World::BuildRaidZoneForMission(mission->id);
    const auto* missionBlueprint = Peter::World::FindMissionBlueprint(mission->id);

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
    if (guidedMode)
    {
      m_eventBus.Emit(Peter::Core::Event{
        Peter::Core::EventCategory::Gameplay,
        "gameplay.onboarding.first_mission.started",
        {{"mission_id", mission->id}}});
    }

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

    const auto raidScene = MeasureMilliseconds([&]() { return sceneShell.LoadRaidZone(raidZone); }, transitionMs);
    EmitPerformanceMetric(
      m_eventBus,
      "performance.scene.transition",
      "transition_ms",
      transitionMs,
      "ms",
      {{"target_scene", raidZone.sceneId}});
    (void)raidScene;
    m_platform.ui->PresentState("ui.raid_hud");
    double raidOverviewRenderMs = 0.0;
    const auto raidOverview = MeasureMilliseconds(
      [&]() { return Peter::UI::RenderRaidZoneOverview(raidZone, *mission); },
      raidOverviewRenderMs);
    EmitPerformanceMetric(
      m_eventBus,
      "performance.ui.panel_render",
      "ui_render_p95_ms",
      raidOverviewRenderMs,
      "ms",
      {{"panel_id", "raid.overview"}});
    m_platform.ui->PresentPanel("raid.overview", raidOverview);
    if (missionBlueprint != nullptr)
    {
      m_platform.ui->PresentPanel(
        "content.preview.mission_graph",
        Peter::UI::RenderMissionBlueprintPreview(*missionBlueprint, raidZone));
      if (!missionBlueprint->encounterPatternIds.empty())
      {
        if (const auto* encounter = Peter::World::FindEncounterPattern(missionBlueprint->encounterPatternIds.front()))
        {
          m_platform.ui->PresentPanel(
            "content.preview.encounter",
            Peter::UI::RenderEncounterPatternPreview(*encounter));
        }
      }
    }

    Peter::Core::RaidSessionState raid;
    raid.missionId = mission->id;
    raid.missionTemplateId = mission->templateType;
    raid.sceneId = raidZone.sceneId;
    raid.currentRoomId = raidZone.entryRoomId;
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
    const std::string firstEncounterRoomId = raidZone.encounters.empty()
      ? raidZone.entryRoomId
      : raidZone.encounters.front().roomId;
    const std::string secondEncounterRoomId = raidZone.encounters.size() > 1
      ? raidZone.encounters.at(1).roomId
      : firstEncounterRoomId;
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
    combatContext.distanceToThreatMeters = mission->templateType == "timed_extraction" ? 7 : 4;
    combatContext.companionHealthPercent = expectSuccess ? 92 : 41;
    combatContext.urgencyLevel = mission->templateType == "timed_extraction" ? 3 : 2;
    combatContext.roomNodeId = firstEncounterRoomId;
    combatContext.routeNodeId = raidZone.sceneId + ".entry_loop";
    combatContext.currentGoal = mission->templateType == "recover_artifact"
      ? "goal.recover_artifact_objective"
      : (mission->templateType == "escort_companion" ? "goal.defend_player" : "goal.follow_player");
    combatContext.currentTargetId = "player";
    combatContext.visibleThreatId = raidZone.encounters.at(0).enemies.front().enemyId;
    combatContext.lastKnownThreatPositionToken = firstEncounterRoomId;
    combatContext.interestMarkerActive = combatContext.rareLootVisible;
    combatContext.interestMarkerId = combatContext.rareLootVisible ? ("marker.rare_loot." + firstEncounterRoomId) : "";

    auto companionDecision = evaluateCompanion(state.companionConfig, combatContext);
    postFeedback("companion_ack", companionDecision.calloutToken, 1, false, "companion_decision");
    EmitCompanionDecision(firstEncounterRoomId, companionDecision);

    Peter::Combat::EncounterRequest encounterRequest{
      raidZone.encounters.at(0).enemies,
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
    if (encounterOutcome.alarmTriggered)
    {
      postFeedback("threat_escalation", "alarm_triggered", 3, true, "combat_alarm");
    }
    raid.playerHealth = std::max(0, raid.playerHealth - encounterOutcome.playerDamage + encounterOutcome.playerHealing);
    raid.timeline.push_back("Combat: " + encounterOutcome.summary);

    if (mission->templateType == "timed_extraction" || mission->templateType == "escort_companion")
    {
      combatContext.playerLowHealth = !expectSuccess;
      combatContext.playerNeedsRevive = mission->templateType == "escort_companion" &&
        HasNode(state.workshop, "track.companion_capabilities.repair_pulse");
      combatContext.distanceToPlayerMeters = 3;
      combatContext.distanceToThreatMeters = mission->templateType == "timed_extraction" ? 6 : 4;
      combatContext.roomNodeId = secondEncounterRoomId;
      combatContext.routeNodeId = raidZone.sceneId + ".mid_path";
      combatContext.currentGoal = mission->templateType == "escort_companion"
        ? "goal.defend_player"
        : "goal.scout_path";
      combatContext.visibleThreatId = raidZone.encounters.at(1).enemies.front().enemyId;
      combatContext.lastKnownThreatPositionToken = secondEncounterRoomId;
      companionDecision = evaluateCompanion(state.companionConfig, combatContext);
      postFeedback("companion_ack", companionDecision.calloutToken, 1, false, "companion_decision");
      EmitCompanionDecision(secondEncounterRoomId, companionDecision);
      encounterRequest.enemies = raidZone.encounters.at(1).enemies;
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
      if (encounterOutcome.alarmTriggered)
      {
        postFeedback("threat_escalation", "alarm_triggered", 3, true, "combat_alarm");
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
          postFeedback(
            definition->audioCueFamily,
            std::string(Peter::Inventory::ToString(definition->rarity)),
            2,
            false,
            "loot_pickup");
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
      combatContext.distanceToPlayerMeters = 2;
      combatContext.distanceToThreatMeters = 8;
      combatContext.extractionUrgency = state.accessibility.reducedTimePressure ? 2 : 3;
      combatContext.roomNodeId = raidZone.extractionRoomId;
      combatContext.routeNodeId = "route.machine_silo.vault_watch";
      combatContext.currentGoal = "goal.reach_extraction";
      combatContext.visibleThreatId = "enemy.none";
      combatContext.heardEventToken = "sound.extraction_alarm";
      combatContext.interestMarkerActive = true;
      combatContext.interestMarkerId = "marker.extraction.pad";
      companionDecision = evaluateCompanion(state.companionConfig, combatContext);
      postFeedback("companion_ack", companionDecision.calloutToken, 1, false, "companion_decision");
      EmitCompanionDecision(raidZone.extractionRoomId, companionDecision);

      m_platform.audio->PostWorldCue(raidZone.extraction.successCueId);
      postFeedback("extraction", "success", 3, true, "extraction_success");
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
        if (unlockResult.unlocked)
        {
          postFeedback("workshop_success", nextNode, 2, false, "workshop_unlock");
        }
      }
      m_platform.ui->PresentPanel("workbench.tracks", workshopPanel);

      VisitStation(FindStation("station.home.companion"));
      m_platform.ui->PresentPanel(
        "companion.editor",
        Peter::UI::RenderCompanionBehaviorEditor(state.companionConfig));
      m_platform.ui->PresentPanel("companion.explain", Peter::UI::RenderCompanionExplainPanel(companionDecision));
      m_platform.ui->PresentPanel(
        "companion.debug",
        Peter::UI::RenderAiDebugPanel(Peter::AI::BuildExplainSnapshot(companionDecision)));
      m_eventBus.Emit(Peter::Core::Event{
        Peter::Core::EventCategory::CreatorTools,
        "creator_tools.explain_panel.opened",
        {
            {"action", companionDecision.lastAction},
            {"state", companionDecision.currentState}
          }});
      if (guidedMode)
      {
        m_eventBus.Emit(Peter::Core::Event{
          Peter::Core::EventCategory::Gameplay,
          "gameplay.onboarding.first_explain_panel.completed",
          {{"mission_id", mission->id}}});
      }

      if (guidedMode)
      {
        const auto previousConfig = state.companionConfig;
        const auto previewConfig = Peter::Workshop::BuildBehaviorPreviewConfig(
          previousConfig,
          "stance.guardian",
          {"chip.stay_near_me", "chip.protect_me_first", "chip.help_at_extraction"},
          {{"chip.stay_near_me", 4.0}});
        const auto preview = Peter::Workshop::BuildCompanionBehaviorPreview(previousConfig, previewConfig);
        m_platform.ui->PresentPanel(
          "companion.rule_preview",
          Peter::UI::RenderCreatorValidationMessage(preview.valid, preview.summary + "\n" + preview.deltaSummary + "\n" + preview.comparisonSummary));
        m_eventBus.Emit(Peter::Core::Event{
          Peter::Core::EventCategory::CreatorTools,
          "creator_tools.behavior_chip.previewed",
          {
            {"preview_label", preview.previewLabel},
            {"valid", preview.valid ? "true" : "false"}
          }});
        if (preview.valid)
        {
          state.companionConfig = previewConfig;
          postFeedback("creator_success", "behavior_preview", 2, false, "creator_preview");
          m_eventBus.Emit(Peter::Core::Event{
            Peter::Core::EventCategory::CreatorTools,
            "creator_tools.behavior_chip.applied",
            {
              {"stance_id", state.companionConfig.stanceId},
              {"valid", "true"}
            }});
        }
        const auto previewContext = BuildExplainPreviewContext(state.workshop);
        const auto beforePreview = evaluateCompanion(previousConfig, previewContext);
        const auto afterPreview = evaluateCompanion(state.companionConfig, previewContext);
        m_platform.ui->PresentPanel(
          "companion.rule_effect",
          Peter::UI::RenderCompanionCompareView(beforePreview, afterPreview, preview.deltaSummary));
        m_eventBus.Emit(Peter::Core::Event{
          Peter::Core::EventCategory::CreatorTools,
          "creator_tools.comparison_view.opened",
          {
            {"after_state", afterPreview.currentState},
            {"before_state", beforePreview.currentState}
          }});
        state.ruleEditComplete = preview.valid;
        if (preview.valid)
        {
          m_eventBus.Emit(Peter::Core::Event{
            Peter::Core::EventCategory::Gameplay,
            "gameplay.onboarding.first_creator_change.completed",
            {{"mission_id", mission->id}}});
        }

        RunLesson(state, "lesson.phase4.change_value", false);
        RunLesson(state, "lesson.phase4.change_rule", false);
        RunLesson(state, "lesson.phase4.read_explanation", false);
        RunLesson(state, "lesson.phase4.first_script", false);

        const Peter::Workshop::MiniMissionDraftDefinition miniMissionDraft{
          "mini_mission.creator.machine_silo_intro",
          "Machine Silo Creator Run",
          "A profile-local mini mission built from safe Machine Silo bundles.",
          "bundle.machine_silo.entry_lane",
          "item.salvage.scrap_metal",
          "enemy_group.machine_silo.patrol_pair",
          "room.raid.extraction_pad",
          "reward.creator.scrap_bundle",
          true,
          1};
        const auto miniMissionValidation = Peter::Validation::ValidateMiniMissionDraftDefinition(miniMissionDraft);
        const int miniMissionRevision = m_creatorContentStore.WriteArtifact(
          Peter::Core::CreatorContentKind::MiniMission,
          miniMissionDraft.id,
          Peter::Core::StructuredFields{
            {"display_name", miniMissionDraft.displayName},
            {"enemy_group_id", miniMissionDraft.enemyGroupId},
            {"extraction_point_id", miniMissionDraft.extractionPointId},
            {"loot_goal_item_id", miniMissionDraft.lootGoalItemId},
            {"reward_bundle_id", miniMissionDraft.rewardBundleId},
            {"room_bundle_id", miniMissionDraft.roomBundleId},
            {"summary", miniMissionDraft.summary}});
        (void)Peter::Workshop::ActivateCreatorArtifact(
          state.creatorManifest,
          "mini_mission",
          miniMissionDraft.id,
          miniMissionRevision,
          miniMissionValidation.valid);
        RunLesson(state, "lesson.phase4.build_mini_mission", false);

        state.creatorProgress.safeSimulationRuns += 1;
        state.creatorProgress.mentorViewUnlocked = Peter::Progression::HasUnlockedNode(
          state.workshop,
          "track.creator_unlocks.mentor_view");

        const auto creatorContext = BuildExplainPreviewContext(state.workshop);
        const auto* creatorRuleset = LoadActiveLogicRuleset(m_creatorContentStore, state.creatorManifest);
        const auto creatorScript = LoadActiveTinyScript(m_creatorContentStore, state.creatorManifest);
        const auto creatorConfig = Peter::Workshop::ApplyTinkerValues(previousConfig, state.tinkerValues, true);
        auto creatorOverrides = Peter::Workshop::BuildTinkerBehaviorOverrides(state.tinkerValues, true);
        if (creatorRuleset != nullptr)
        {
          const auto logicOverrides = Peter::Workshop::CompileLogicRuleset(*creatorRuleset, creatorContext);
          creatorOverrides.overrides.insert(
            creatorOverrides.overrides.end(),
            logicOverrides.overrides.begin(),
            logicOverrides.overrides.end());
        }
        if (!creatorScript.id.empty())
        {
          const auto scriptOverrides = Peter::Workshop::BuildScriptBehaviorOverrides(creatorScript, creatorContext);
          creatorOverrides.overrides.insert(
            creatorOverrides.overrides.end(),
            scriptOverrides.overrides.begin(),
            scriptOverrides.overrides.end());
        }
        const auto creatorBefore = evaluateCompanion(previousConfig, creatorContext);
        const auto creatorAfter = evaluateCompanion(creatorConfig, creatorContext, &creatorOverrides);
        m_platform.ui->PresentPanel(
          "creator.safe_simulation.compare",
          Peter::UI::RenderCompanionCompareView(
            creatorBefore,
            creatorAfter,
            "Safe simulation after tinker, logic, and tiny script changes."));
        Peter::Tools::DeterministicScenarioHarness creatorHarness("scenario.ai.loot_vs_safety.v1");
        const auto creatorCompare = creatorHarness.Compare(previousConfig, creatorConfig);
        const auto creatorReplay = Peter::Workshop::BuildCreatorReplaySnippet(
          creatorCompare.scenarioId,
          creatorCompare.beforeActionPath,
          creatorCompare.afterActionPath,
          "Your creator changes produced a new explainable behavior path.");
        m_platform.ui->PresentReplayTimeline(
          "creator.safe_simulation.replay",
          Peter::UI::RenderReplaySnippet(creatorReplay));
        const auto mentorPanel = Peter::UI::RenderMentorSummary(
          state.creatorManifest,
          state.creatorProgress,
          Peter::AI::BuildExplainSnapshot(creatorAfter));
        const auto mentorPath = m_creatorContentStore.WriteMentorSummary(
          "mentor.phase4.creator_summary",
          mentorPanel);
        m_platform.ui->PresentMentorSummaryPrompt(mentorPath.string(), mentorPanel);
        m_eventBus.Emit(Peter::Core::Event{
          Peter::Core::EventCategory::CreatorTools,
          "creator_tools.mentor_view.opened",
          {{"export_path", mentorPath.string()}, {"safe_sim_runs", std::to_string(state.creatorProgress.safeSimulationRuns)}}});
      }

      if (guidedMode)
      {
        state.guidedFirstRunComplete = true;
        m_eventBus.Emit(Peter::Core::Event{
          Peter::Core::EventCategory::Gameplay,
          "gameplay.onboarding.first_extraction.completed",
          {{"mission_id", mission->id}}});
      }

      state.completedRaids += 1;
      state.lastRaidResult = "success";
      state.lastMissionId = mission->id;
      state.lastSceneId = raidZone.sceneId;
      const auto resultsScene = MeasureMilliseconds(
        [&]() { return sceneShell.LoadRaidResults(raid.missionId, true); },
        transitionMs);
      EmitPerformanceMetric(
        m_eventBus,
        "performance.scene.transition",
        "transition_ms",
        transitionMs,
        "ms",
        {{"target_scene", "scene.results.success"}});
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

      m_platform.audio->PostWorldCue(raidZone.extraction.failCueId);
      postFeedback("extraction", "failure", 3, true, "extraction_failure");
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
        m_eventBus.Emit(Peter::Core::Event{
          Peter::Core::EventCategory::Gameplay,
          "gameplay.onboarding.first_repair.completed",
          {{"mission_id", mission->id}}});
      }

      RunLesson(state, "lesson.phase2.first_repair", true);
      state.lastRaidResult = "failure";
      state.lastMissionId = mission->id;
      state.lastSceneId = raidZone.sceneId;
      const auto resultsScene = MeasureMilliseconds(
        [&]() { return sceneShell.LoadRaidResults(raid.missionId, false); },
        transitionMs);
      EmitPerformanceMetric(
        m_eventBus,
        "performance.scene.transition",
        "transition_ms",
        transitionMs,
        "ms",
        {{"target_scene", "scene.results.failure"}});
      (void)resultsScene;
    }

    double summaryRenderMs = 0.0;
    const auto summaryPanel = MeasureMilliseconds(
      [&]() { return Peter::UI::RenderPostRaidSummary(raidSummary); },
      summaryRenderMs);
    EmitPerformanceMetric(
      m_eventBus,
      "performance.ui.panel_render",
      "ui_render_p95_ms",
      summaryRenderMs,
      "ms",
      {{"panel_id", "raid.summary"}});
    m_platform.ui->PresentPanel("raid.summary", summaryPanel);
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
    if (guidedMode)
    {
      m_eventBus.Emit(Peter::Core::Event{
        Peter::Core::EventCategory::Gameplay,
        "gameplay.onboarding.first_mission.completed",
        {{"mission_id", raid.missionId}, {"result", raidSummary.success ? "success" : "failure"}}});
    }

    SavePersistentState(state);

    return SliceRunReport{
      raidSummary.success,
      raidSummary.success
        ? "Completed the Phase 6 quality beta loop: budgets, onboarding telemetry, hardened saves, and polished creator feedback are active."
        : "Completed the Phase 6 failure-and-recovery loop: save safety, creator containment, and post-raid clarity remained intact under failure.",
      mission->id,
      companionDecision,
      Peter::Workshop::BuildCompanionBehaviorPreview(state.companionConfig, state.companionConfig),
      extractionResult,
      raidSummary};
  }
} // namespace Peter::App
