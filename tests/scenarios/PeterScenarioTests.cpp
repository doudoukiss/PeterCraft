#include "PeterAI/CompanionAi.h"
#include "PeterCombat/EncounterSimulator.h"
#include "PeterInventory/InventoryState.h"
#include "PeterProgression/Crafting.h"
#include "PeterTest/TestMacros.h"
#include "PeterTools/ScenarioHarness.h"
#include "PeterValidation/ValidationModule.h"
#include "PeterWorld/SliceContent.h"

PETER_TEST_MAIN({
  Peter::Tools::DeterministicScenarioHarness harness("scenario.phase2.systems_alpha");
  const auto report = harness.Run({
    "boot",
    "mission_choice",
    "salvage_run",
    "post_raid_summary",
    "failure_recovery",
    "lesson_replay"
  });

  PETER_ASSERT_EQ(std::string("scenario.phase2.systems_alpha"), report.scenarioId);
  PETER_ASSERT_EQ(6, report.stepCount);

  const auto* artifactMission = Peter::World::FindMissionTemplate("mission.recover_artifact.machine_silo");
  PETER_ASSERT_TRUE(artifactMission != nullptr);
  PETER_ASSERT_EQ(std::string("recover_artifact"), artifactMission->templateType);
  PETER_ASSERT_TRUE(!artifactMission->sideObjectives.empty());

  const auto escortDecision = Peter::AI::EvaluateCompanion(
    Peter::AI::CompanionConfig{6.0, false},
    Peter::AI::CompanionWorldContext{
      true,
      true,
      false,
      false,
      false,
      true,
      false,
      false,
      true,
      true,
      false,
      4});
  PETER_ASSERT_EQ(std::string("support_guard"), escortDecision.currentState);

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

  Peter::Inventory::AddToLedger(inventory.stash, "item.salvage.scrap_metal", 2);
  std::string recoverySummary;
  PETER_ASSERT_TRUE(Peter::Inventory::ReclaimFavoriteItem(recoveryState, inventory, loadout, &recoverySummary));

  const auto lesson = Peter::World::FindTutorialLesson("lesson.phase2.first_repair");
  PETER_ASSERT_TRUE(lesson != nullptr);
  PETER_ASSERT_TRUE(Peter::Validation::ValidateTutorialLesson(*lesson).valid);
})
