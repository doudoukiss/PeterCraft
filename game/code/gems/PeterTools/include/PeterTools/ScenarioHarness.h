#pragma once

#include <initializer_list>
#include <string>

namespace Peter::Tools
{
  struct ScenarioReport
  {
    std::string scenarioId;
    int stepCount = 0;
  };

  class DeterministicScenarioHarness
  {
  public:
    explicit DeterministicScenarioHarness(std::string scenarioId);
    [[nodiscard]] ScenarioReport Run(std::initializer_list<std::string> steps) const;

  private:
    std::string m_scenarioId;
  };
} // namespace Peter::Tools
