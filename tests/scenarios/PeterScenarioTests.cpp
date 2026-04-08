#include "PeterAI/CompanionAi.h"
#include "PeterTest/TestMacros.h"
#include "PeterTools/ScenarioHarness.h"

PETER_TEST_MAIN({
  Peter::Tools::DeterministicScenarioHarness harness("scenario.phase1.rule_edit_preview");
  const auto report = harness.Run({"boot", "home_base", "raid_success", "rule_edit", "next_raid_preview"});

  PETER_ASSERT_EQ(std::string("scenario.phase1.rule_edit_preview"), report.scenarioId);
  PETER_ASSERT_EQ(5, report.stepCount);

  const auto beforeEdit = Peter::AI::EvaluateCompanion(
    Peter::AI::CompanionConfig{6.0, false},
    Peter::AI::CompanionWorldContext{false, false, false, false, false, false, true, 8});
  const auto afterEdit = Peter::AI::EvaluateCompanion(
    Peter::AI::CompanionConfig{9.0, false},
    Peter::AI::CompanionWorldContext{false, false, false, false, false, false, true, 8});
  PETER_ASSERT_EQ(std::string("regroup"), beforeEdit.currentState);
  PETER_ASSERT_EQ(std::string("support_loot"), afterEdit.currentState);
})
