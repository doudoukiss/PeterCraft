#pragma once

#include "PeterAI/CompanionAi.h"

#include <string>
#include <vector>

namespace Peter::Tools
{
  struct ScenarioReport
  {
    std::string scenarioId;
    int stepCount = 0;
    bool deterministic = false;
    bool passed = false;
    std::vector<std::string> actualActionPath;
    std::vector<std::string> actualStatePath;
  };

  struct ScenarioCompareReport
  {
    std::string scenarioId;
    bool changed = false;
    std::vector<std::string> beforeActionPath;
    std::vector<std::string> afterActionPath;
  };

  class DeterministicScenarioHarness
  {
  public:
    explicit DeterministicScenarioHarness(std::string scenarioId);
    [[nodiscard]] ScenarioReport Run() const;
    [[nodiscard]] ScenarioCompareReport Compare(
      const Peter::AI::CompanionConfig& beforeConfig,
      const Peter::AI::CompanionConfig& afterConfig) const;

  private:
    std::string m_scenarioId;
  };
} // namespace Peter::Tools
