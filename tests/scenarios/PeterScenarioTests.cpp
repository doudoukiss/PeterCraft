#include "PeterTest/TestMacros.h"
#include "PeterTools/ScenarioHarness.h"

PETER_TEST_MAIN({
  Peter::Tools::DeterministicScenarioHarness harness("scenario.foundation.debug_overlay");
  const auto report = harness.Run({"boot", "profile", "scene_load"});

  PETER_ASSERT_EQ(std::string("scenario.foundation.debug_overlay"), report.scenarioId);
  PETER_ASSERT_EQ(3, report.stepCount);
})
