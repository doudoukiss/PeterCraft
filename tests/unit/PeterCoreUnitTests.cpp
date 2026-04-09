#include "PeterAI/CompanionAi.h"
#include "PeterAdapters/PlatformServices.h"
#include "PeterCombat/EncounterSimulator.h"
#include "PeterCore/EventBus.h"
#include "PeterCore/FeatureRegistry.h"
#include "PeterCore/QualityProfile.h"
#include "PeterCore/StableId.h"
#include "PeterInventory/InventoryState.h"
#include "PeterProgression/Crafting.h"
#include "PeterTest/TestMacros.h"
#include "PeterTelemetry/QualityMetrics.h"
#include "PeterTraversal/TraversalProfile.h"
#include "PeterUI/SlicePresentation.h"
#include "PeterValidation/ValidationModule.h"
#include "PeterWorkshop/CreatorWorkshop.h"
#include "PeterWorkshop/WorkshopTuning.h"
#include "PeterWorld/SliceContent.h"

namespace
{
  class CountingSink final : public Peter::Core::IEventSink
  {
  public:
    void Consume(const Peter::Core::Event&) override
    {
      ++count;
    }

    int count = 0;
  };
} // namespace

PETER_TEST_MAIN({
  PETER_ASSERT_TRUE(Peter::Core::StableId::IsValid("item.salvage.scrap_metal"));
  PETER_ASSERT_TRUE(!Peter::Core::StableId::IsValid("Item.Salvage.Bad"));

  Peter::Core::FeatureRegistry registry({"0.4.0", "phase3"});
  registry.SetFlag("feature.ai_alpha", true);
  PETER_ASSERT_TRUE(registry.IsEnabled("feature.ai_alpha"));
  PETER_ASSERT_TRUE(registry.Flags().contains("feature.ai_alpha"));
  PETER_ASSERT_EQ(std::string("phase3"), registry.Version().track);

  Peter::Adapters::RuntimeMode runtimeMode = Peter::Adapters::RuntimeMode::Headless;
  PETER_ASSERT_TRUE(Peter::Adapters::TryParseRuntimeMode("playable", runtimeMode));
  PETER_ASSERT_EQ(std::string("playable"), Peter::Adapters::ToString(runtimeMode));
  const auto runtimeDescriptor = Peter::Adapters::BuildRuntimeDescriptor(runtimeMode, false);
  PETER_ASSERT_EQ(std::string("playable_stub"), runtimeDescriptor.backendId);

  Peter::Core::EventBus eventBus;
  CountingSink sink;
  eventBus.RegisterSink(&sink);
  eventBus.Emit({Peter::Core::EventCategory::Gameplay, "gameplay.test", {}});
  PETER_ASSERT_EQ(1, sink.count);

  const auto qualityProfile = Peter::Core::LoadPhase6QualityProfile();
  PETER_ASSERT_TRUE(Peter::Validation::ValidatePhase6QualityProfile(qualityProfile).valid);
  PETER_ASSERT_EQ(60, qualityProfile.budgets.fpsTarget);
  const auto playableQualityProfile = Peter::Core::LoadPhase7PlayableQualityProfile();
  PETER_ASSERT_TRUE(Peter::Validation::ValidatePhase7PlayableQualityProfile(playableQualityProfile).valid);
  PETER_ASSERT_EQ(85.0, playableQualityProfile.budgets.inputToMotionLatencyBudgetMs);
  const auto traversalProfile = Peter::Traversal::BuildTraversalProfile(qualityProfile);
  PETER_ASSERT_TRUE(Peter::Traversal::EvaluateMovementResponsivenessScore(traversalProfile) > 0.0);
  const auto qualityReport = Peter::Telemetry::EvaluateQualityReport(
    qualityProfile,
    {
      {"cold_boot_ms", "ms", 1000.0, 0.0, true, "test"},
      {"fps_average", "fps", 60.0, 0.0, true, "test"},
      {"frame_time_p95_ms", "ms", 16.6, 0.0, true, "test"}
    });
  PETER_ASSERT_TRUE(qualityReport.passed);
  const auto playableQualityReport = Peter::Telemetry::EvaluateQualityReport(
    playableQualityProfile,
    {
      {"input_to_motion_latency_ms", "ms", 0.0, 0.0, true, "test", false}
    });
  PETER_ASSERT_TRUE(playableQualityReport.passed);
  PETER_ASSERT_EQ(1, static_cast<int>(playableQualityReport.unmeasuredSamples));

  PETER_ASSERT_TRUE(Peter::AI::HasLineOfSight(6, 10, false));
  PETER_ASSERT_TRUE(!Peter::AI::HasLineOfSight(12, 10, false));
  PETER_ASSERT_TRUE(Peter::AI::CanHearEvent(5, 7, 3));
  PETER_ASSERT_TRUE(!Peter::AI::CanHearEvent(1, 8, 2));

  const auto decayedMemory = Peter::AI::DecayThreatMemory(
    {Peter::AI::ThreatMemoryEntry{"enemy.test", "room.raid.guard_post", 0, 2, 0.9}},
    2);
  PETER_ASSERT_EQ(1, static_cast<int>(decayedMemory.size()));
  PETER_ASSERT_EQ(2, decayedMemory.front().ageTurns);

  Peter::AI::CompanionWorldContext reviveContext;
  reviveContext.threatVisible = true;
  reviveContext.sameTargetMarked = true;
  reviveContext.playerNeedsRevive = true;
  reviveContext.playerLowHealth = true;
  reviveContext.unsafeToAdvance = true;
  reviveContext.repairPulseUnlocked = true;
  reviveContext.distanceToPlayerMeters = 2;
  reviveContext.distanceToThreatMeters = 4;
  reviveContext.roomNodeId = "room.raid.guard_post";
  reviveContext.routeNodeId = "route.machine_silo.vault_watch";
  reviveContext.currentGoal = "goal.defend_player";
  reviveContext.visibleThreatId = "enemy.machine_patrol.chaser_02";
  reviveContext.lastKnownThreatPositionToken = "room.raid.guard_post";

  const auto guardianDecision = Peter::AI::EvaluateCompanion(
    Peter::AI::CompanionConfig{
      6.0,
      false,
      "stance.guardian",
      {"chip.stay_near_me", "chip.protect_me_first"},
      {{"chip.stay_near_me", 6.0}, {"chip.protect_me_first", 1.0}}},
    reviveContext);
  PETER_ASSERT_EQ(std::string("revive"), guardianDecision.lastAction);
  PETER_ASSERT_EQ(std::string("revive_help"), guardianDecision.currentState);
  PETER_ASSERT_TRUE(!guardianDecision.topCandidates.empty());

  const auto explainSnapshot = Peter::AI::BuildExplainSnapshot(guardianDecision);
  PETER_ASSERT_EQ(std::string("goal.revive_player"), explainSnapshot.currentGoal);
  PETER_ASSERT_TRUE(!explainSnapshot.topReason.empty());

  Peter::AI::CompanionWorldContext lootContext;
  lootContext.rareLootVisible = false;
  lootContext.lootPingUnlocked = true;
  lootContext.distanceToPlayerMeters = 7;
  lootContext.distanceToThreatMeters = 6;
  lootContext.roomNodeId = "room.raid.patrol_hall";
  lootContext.routeNodeId = "route.machine_silo.entry_loop";
  lootContext.currentGoal = "goal.follow_player";
  lootContext.interestMarkerActive = true;
  lootContext.interestMarkerId = "marker.rare_loot.vault_cache";

  const auto beforeEdit = Peter::AI::EvaluateCompanion(Peter::AI::DefaultCompanionConfig(), lootContext);
  const auto previewConfig = Peter::Workshop::BuildBehaviorPreviewConfig(
    Peter::AI::DefaultCompanionConfig(),
    "stance.scavenger",
    {"chip.stay_near_me", "chip.grab_rare_loot"},
    {{"chip.stay_near_me", 8.0}, {"chip.grab_rare_loot", 1.0}});
  const auto afterEdit = Peter::AI::EvaluateCompanion(previewConfig, lootContext);
  PETER_ASSERT_EQ(std::string("move_to"), beforeEdit.lastAction);
  PETER_ASSERT_EQ(std::string("loot"), afterEdit.lastAction);

  const auto preview = Peter::Workshop::BuildCompanionBehaviorPreview(
    Peter::AI::DefaultCompanionConfig(),
    previewConfig);
  PETER_ASSERT_TRUE(preview.valid);
  PETER_ASSERT_TRUE(Peter::Validation::ValidateTinkerVariableDefinition(
    Peter::Workshop::BuildPhase4TinkerVariables().front()).valid);
  PETER_ASSERT_TRUE(Peter::Validation::ValidateTinkerPresetDefinition(
    Peter::Workshop::BuildPhase4TinkerPresets().front()).valid);
  PETER_ASSERT_TRUE(Peter::Validation::ValidateLogicRulesetDefinition(
    Peter::Workshop::BuildPhase4LogicTemplates().front()).valid);
  const auto scriptValidation = Peter::Workshop::ValidateTinyScript(
    Peter::Workshop::BuildPhase4TinyScriptTemplates().front());
  PETER_ASSERT_TRUE(scriptValidation.valid);
  PETER_ASSERT_TRUE(Peter::Validation::ValidateTinyScriptDefinition(
    Peter::Workshop::BuildPhase4TinyScriptTemplates().front()).valid);
  PETER_ASSERT_TRUE(Peter::Validation::ValidateMiniMissionDraftDefinition(
    Peter::Workshop::MiniMissionDraftDefinition{
      "mini_mission.creator.machine_silo_intro",
      "Machine Silo Creator Run",
      "Profile-local mini mission.",
      "bundle.machine_silo.entry_lane",
      "item.salvage.scrap_metal",
      "enemy_group.machine_silo.patrol_pair",
      "room.raid.extraction_pad",
      "reward.creator.scrap_bundle",
      true,
      1}).valid);

  const auto configValidation = Peter::Validation::ValidateCompanionConfig(previewConfig);
  PETER_ASSERT_TRUE(configValidation.valid);
  PETER_ASSERT_TRUE(Peter::Validation::ValidateBehaviorChipDefinition(
    Peter::AI::BuildPhase3BehaviorChips().front()).valid);
  PETER_ASSERT_TRUE(Peter::Validation::ValidateBehaviorStanceDefinition(
    Peter::AI::BuildPhase3Stances().front()).valid);
  PETER_ASSERT_TRUE(Peter::Validation::ValidateEnemyArchetypeDefinition(
    Peter::AI::BuildPhase3EnemyArchetypes().front()).valid);
  PETER_ASSERT_TRUE(Peter::Validation::ValidatePatrolRouteDefinition(
    Peter::AI::BuildPhase3PatrolRoutes().front()).valid);
  PETER_ASSERT_TRUE(Peter::Validation::ValidateAiScenarioDefinition(
    Peter::AI::BuildPhase3AiScenarios().front()).valid);

  Peter::Inventory::InventoryState inventory;
  Peter::Inventory::LoadoutState loadout;
  loadout.favoriteItemId = loadout.equippedToolId;
  for (const auto& itemId : Peter::Inventory::EquippedItemIds(loadout))
  {
    const auto* definition = Peter::Inventory::FindItemDefinition(itemId);
    if (definition != nullptr && definition->repairable)
    {
      inventory.equippedDurability[itemId] = definition->maxDurability;
    }
  }

  std::string denialReason;
  PETER_ASSERT_TRUE(Peter::Inventory::TryAddCarriedItem(
    inventory,
    loadout,
    "item.salvage.scrap_metal",
    3,
    &denialReason));
  PETER_ASSERT_EQ(1, Peter::Inventory::UsedCarrySlots(inventory));

  Peter::Combat::CombatantState combatant{"player", 40, 100};
  const auto directHit = Peter::Combat::ResolveDamageAction(
    combatant,
    Peter::Combat::DamageSpec{
      "test.direct",
      "enemy.test",
      "player",
      Peter::Combat::CombatActionKind::DirectHit,
      10,
      {"machine"},
      {Peter::Combat::StatusEffectSpec{"overheat", 1, 3}}});
  PETER_ASSERT_EQ(10, directHit.damageApplied);
  PETER_ASSERT_EQ(30, combatant.health);

  const auto statusTicks = Peter::Combat::TickStatuses(combatant, "system.status");
  PETER_ASSERT_EQ(1, static_cast<int>(statusTicks.size()));
  PETER_ASSERT_EQ(3, statusTicks.front().damageApplied);

  Peter::Inventory::RecoveryState recoveryState;
  inventory.equippedDurability[loadout.equippedToolId] = 20;
  const auto failureRecovery =
    Peter::Inventory::ApplyRaidFailureRecovery(inventory, loadout, recoveryState, false);
  PETER_ASSERT_TRUE(!failureRecovery.favoriteItemMovedToRecovery.empty());
  Peter::Inventory::AddToLedger(inventory.stash, "item.salvage.scrap_metal", 2);
  std::string recoverySummary;
  PETER_ASSERT_TRUE(Peter::Inventory::ReclaimFavoriteItem(recoveryState, inventory, loadout, &recoverySummary));

  Peter::Inventory::AddToLedger(inventory.stash, "item.salvage.scrap_metal", 3);
  Peter::Progression::WorkshopState workshopState;
  const auto unlockResult = Peter::Progression::UnlockUpgradeNode(
    "track.inventory_capacity.salvage_pouch",
    inventory,
    loadout,
    workshopState);
  PETER_ASSERT_TRUE(unlockResult.unlocked);

  Peter::UI::AccessibilitySettings settings;
  settings.actionBindings["action.jump"] = "Space";
  settings.subtitleBackgroundEnabled = true;
  settings.highContrastEnabled = true;
  settings.iconRedundancyEnabled = true;
  settings.motionComfortEnabled = true;
  const auto serializedSettings = Peter::UI::ToSaveFields(settings);
  const auto loadedSettings = Peter::UI::AccessibilitySettingsFromSaveFields(
    serializedSettings,
    {
      Peter::Adapters::ActionBinding{"action.jump", "Space", "A", "input.jump", "movement", true, false},
      Peter::Adapters::ActionBinding{"action.interact", "E", "X", "input.interact", "interaction", true, true}
    });
  PETER_ASSERT_EQ(100, loadedSettings.textScalePercent);
  PETER_ASSERT_TRUE(loadedSettings.highContrastEnabled);
  PETER_ASSERT_TRUE(Peter::Validation::ValidateAccessibilitySettings(loadedSettings, qualityProfile).valid);

  Peter::Workshop::CreatorManifest creatorManifest;
  const auto activation = Peter::Workshop::ActivateCreatorArtifact(
    creatorManifest,
    "logic",
    "logic.template.protect_player",
    1,
    true);
  PETER_ASSERT_TRUE(activation.success);
  PETER_ASSERT_TRUE(Peter::Validation::ValidateCreatorManifest(creatorManifest).valid);

  const auto creatorFields = Peter::Workshop::ToSaveFields(creatorManifest);
  const auto reloadedManifest = Peter::Workshop::CreatorManifestFromSaveFields(creatorFields);
  PETER_ASSERT_EQ(std::string("logic.template.protect_player"), reloadedManifest.activeDraftIds.at("logic"));

  const auto validMission = Peter::Validation::ValidateMissionTemplate(
    Peter::World::BuildPhase2MissionTemplates().front());
  PETER_ASSERT_TRUE(validMission.valid);
  PETER_ASSERT_TRUE(!Peter::World::BuildPhase5RoomKits().empty());
  PETER_ASSERT_TRUE(!Peter::World::BuildPhase5RoomVariants().empty());
  PETER_ASSERT_TRUE(!Peter::World::BuildPhase5MissionBlueprints().empty());
  PETER_ASSERT_TRUE(Peter::Validation::ValidateRoomKitDefinition(
    Peter::World::BuildPhase5RoomKits().front()).valid);
  PETER_ASSERT_TRUE(Peter::Validation::ValidateRoomVariantDefinition(
    Peter::World::BuildPhase5RoomVariants().front()).valid);
  PETER_ASSERT_TRUE(Peter::Validation::ValidateEncounterPatternDefinition(
    Peter::World::BuildPhase5EncounterPatterns().front()).valid);
  PETER_ASSERT_TRUE(Peter::Validation::ValidateMissionBlueprintDefinition(
    Peter::World::BuildPhase5MissionBlueprints().front()).valid);
  PETER_ASSERT_TRUE(Peter::Validation::ValidateFeedbackTagDefinition(
    Peter::World::BuildPhase5FeedbackTags().front()).valid);
  PETER_ASSERT_TRUE(Peter::Validation::ValidateWorldStyleProfileDefinition(
    Peter::World::BuildPhase5StyleProfiles().front()).valid);
  PETER_ASSERT_TRUE(Peter::Validation::ValidateShippableContentManifest(
    Peter::World::BuildPhase5ShippableContentManifest()).valid);
  const auto roomMetrics = Peter::World::BuildRoomMetricsSummary("room_variant.machine_silo.patrol_hall");
  PETER_ASSERT_EQ(std::string("linear"), roomMetrics.connectorClass);
})
