#pragma once

#include "PeterAI/CompanionAi.h"
#include "PeterCore/SaveDomainStore.h"
#include "PeterTelemetry/QualityMetrics.h"

#include <map>
#include <string>

namespace Peter::Debug
{
  class DebugOverlay
  {
  public:
    void SetValue(const std::string& key, const std::string& value);
    void SetAiSnapshot(const Peter::AI::AgentExplainSnapshot& snapshot);
    void SetSaveHealthReport(const Peter::Core::SaveHealthReport& report);
    void SetQualityReport(const Peter::Telemetry::QualityReport& report);
    [[nodiscard]] std::string Render() const;

  private:
    std::map<std::string, std::string, std::less<>> m_values;
  };
} // namespace Peter::Debug
