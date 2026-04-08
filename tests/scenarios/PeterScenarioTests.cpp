#include "PeterAI/CompanionAi.h"
#include "PeterCombat/EncounterSimulator.h"
#include "PeterInventory/InventoryState.h"
#include "PeterTest/TestMacros.h"
#include "PeterTools/ScenarioHarness.h"
#include "PeterValidation/ValidationModule.h"
#include "PeterWorkshop/CreatorWorkshop.h"
#include "PeterWorld/SliceContent.h"

PETER_TEST_MAIN({
  const std::vector<std::string> scenarioIds = {
    "scenario.ai.follow_corridor.v1",
    "scenario.ai.enemy_ambush.v1",
    "scenario.ai.low_health_retreat.v1",
    "scenario.ai.revive_under_pressure.v1",
    "scenario.ai.extraction_rush.v1",
    "scenario.ai.loot_vs_safety.v1"
  };

  for (const auto& scenarioId : scenarioIds)
  {
    Peter::Tools::DeterministicScenarioHarness harness(scenarioId);
    const auto report = harness.Run();
    PETER_ASSERT_EQ(scenarioId, report.scenarioId);
    PETER_ASSERT_TRUE(report.deterministic);
    PETER_ASSERT_TRUE(report.passed);
    PETER_ASSERT_TRUE(report.stepCount >= 1);
    PETER_ASSERT_EQ(report.stepCount, static_cast<int>(report.actualActionPath.size()));
    PETER_ASSERT_EQ(report.stepCount, static_cast<int>(report.actualStatePath.size()));
  }

  Peter::Tools::DeterministicScenarioHarness compareHarness("scenario.ai.follow_corridor.v1");
  const auto compareReport = compareHarness.Compare(
    Peter::AI::DefaultCompanionConfig(),
    Peter::AI::CompanionConfig{
      6.0,
      true,
      "stance.cautious",
      {"chip.stay_near_me"},
      {{"chip.stay_near_me", 4.0}}});
  PETER_ASSERT_EQ(std::string("scenario.ai.follow_corridor.v1"), compareReport.scenarioId);
  PETER_ASSERT_TRUE(!compareReport.beforeActionPath.empty());
  PETER_ASSERT_EQ(
    static_cast<int>(compareReport.beforeActionPath.size()),
    static_cast<int>(compareReport.afterActionPath.size()));
  const auto replaySnippet = Peter::Workshop::BuildCreatorReplaySnippet(
    compareReport.scenarioId,
    compareReport.beforeActionPath,
    compareReport.afterActionPath,
    "Creator compare replay");
  PETER_ASSERT_TRUE(!replaySnippet.timeline.empty());

  const auto* followScenario = Peter::AI::FindAiScenario("scenario.ai.follow_corridor.v1");
  PETER_ASSERT_TRUE(followScenario != nullptr);
  PETER_ASSERT_TRUE(Peter::Validation::ValidateAiScenarioDefinition(*followScenario).valid);

  const auto* ambushScenario = Peter::AI::FindAiScenario("scenario.ai.enemy_ambush.v1");
  PETER_ASSERT_TRUE(ambushScenario != nullptr);
  PETER_ASSERT_EQ(std::string("attack"), ambushScenario->steps.back().expectedActionId);

  const auto escortMission = Peter::World::FindMissionTemplate("mission.escort_companion.machine_silo");
  PETER_ASSERT_TRUE(escortMission != nullptr);
  PETER_ASSERT_EQ(std::string("escort_companion"), escortMission->templateType);

  Peter::Inventory::InventoryState inventory;
  Peter::Inventory::LoadoutState loadout;
  loadout.favoriteItemId = loadout.equippedToolId;
  inventory.equippedDurability[loadout.equippedToolId] = 20;
  inventory.equippedDurability[loadout.equippedGadgetId] = 100;
  inventory.equippedDurability[loadout.equippedCompanionModuleId] = 100;
  Peter::Inventory::RecoveryState recoveryState;
  const auto recoveryResult =
    Peter::Inventory::ApplyRaidFailureRecovery(inventory, loadout, recoveryState, false);
  PETER_ASSERT_TRUE(!recoveryResult.favoriteItemMovedToRecovery.empty());

  Peter::AI::CompanionWorldContext extractionContext;
  extractionContext.extractionActive = true;
  extractionContext.timedMissionPressure = true;
  extractionContext.playerLowHealth = true;
  extractionContext.distanceToPlayerMeters = 2;
  extractionContext.distanceToThreatMeters = 7;
  extractionContext.companionHealthPercent = 76;
  extractionContext.extractionUrgency = 3;
  extractionContext.urgencyLevel = 3;
  extractionContext.roomNodeId = "room.raid.extraction_pad";
  extractionContext.routeNodeId = "route.machine_silo.vault_watch";
  extractionContext.currentGoal = "goal.reach_extraction";
  extractionContext.currentTargetId = "player";
  extractionContext.heardEventToken = "sound.extraction_alarm";
  extractionContext.interestMarkerActive = true;
  extractionContext.interestMarkerId = "marker.extraction.pad";

  const auto extractionDecision = Peter::AI::EvaluateCompanion(
    Peter::AI::CompanionConfig{
      6.0,
      false,
      "stance.guardian",
      {"chip.stay_near_me", "chip.help_at_extraction"},
      {{"chip.stay_near_me", 6.0}, {"chip.help_at_extraction", 1.0}}},
    extractionContext);
  PETER_ASSERT_EQ(std::string("extract"), extractionDecision.lastAction);
  PETER_ASSERT_EQ(std::string("extract_assist"), extractionDecision.currentState);

  const auto* ruleset = Peter::Workshop::FindLogicTemplate("logic.template.protect_player");
  PETER_ASSERT_TRUE(ruleset != nullptr);
  const auto logicOverrides = Peter::Workshop::CompileLogicRuleset(*ruleset, extractionContext);
  PETER_ASSERT_TRUE(!logicOverrides.overrides.empty());

  const auto scriptResult = Peter::Workshop::RunTinyScript(
    Peter::Workshop::BuildPhase4TinyScriptTemplates().front(),
    extractionContext);
  PETER_ASSERT_TRUE(scriptResult.valid);

  Peter::Combat::EncounterRequest encounterRequest{
    {Peter::AI::BuildEnemyUnit("enemy.machine_patrol.support_01", Peter::AI::EnemyVariant::AlarmSupport, "room.raid.guard_post")},
    extractionDecision,
    false,
    true,
    true,
    60};
  const auto encounterOutcome = Peter::Combat::ResolveEncounter(encounterRequest);
  PETER_ASSERT_TRUE(!encounterOutcome.events.empty());
})
