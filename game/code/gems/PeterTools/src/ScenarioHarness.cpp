#include "PeterTools/ScenarioHarness.h"

#include <utility>

namespace Peter::Tools
{
  DeterministicScenarioHarness::DeterministicScenarioHarness(std::string scenarioId)
    : m_scenarioId(std::move(scenarioId))
  {
  }

  ScenarioReport DeterministicScenarioHarness::Run() const
  {
    ScenarioReport report;
    report.scenarioId = m_scenarioId;

    const auto* scenario = Peter::AI::FindAiScenario(m_scenarioId);
    if (scenario == nullptr)
    {
      return report;
    }

    report.deterministic = scenario->deterministic;
    report.stepCount = static_cast<int>(scenario->steps.size());
    report.passed = true;

    for (const auto& step : scenario->steps)
    {
      if (step.useEnemy)
      {
        const auto snapshot = Peter::AI::EvaluateEnemy(step.enemy, step.worldContext);
        report.actualActionPath.push_back(snapshot.lastAction);
        report.actualStatePath.push_back(snapshot.currentState);
        if (snapshot.lastAction != step.expectedActionId || snapshot.currentState != step.expectedState)
        {
          report.passed = false;
        }
      }
      else
      {
        const auto snapshot = Peter::AI::EvaluateCompanion(step.config, step.worldContext);
        report.actualActionPath.push_back(snapshot.lastAction);
        report.actualStatePath.push_back(snapshot.currentState);
        if (snapshot.lastAction != step.expectedActionId || snapshot.currentState != step.expectedState)
        {
          report.passed = false;
        }
      }
    }

    return report;
  }
} // namespace Peter::Tools
