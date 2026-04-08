#include "PeterTools/ScenarioHarness.h"

#include <utility>

namespace Peter::Tools
{
  DeterministicScenarioHarness::DeterministicScenarioHarness(std::string scenarioId)
    : m_scenarioId(std::move(scenarioId))
  {
  }

  ScenarioReport DeterministicScenarioHarness::Run(const std::initializer_list<std::string> steps) const
  {
    return ScenarioReport{m_scenarioId, static_cast<int>(steps.size())};
  }
} // namespace Peter::Tools
