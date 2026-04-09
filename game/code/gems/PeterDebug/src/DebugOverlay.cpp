#include "PeterDebug/DebugOverlay.h"

#include <sstream>

namespace Peter::Debug
{
  namespace
  {
    std::string FormatFeatureFlags(const std::map<std::string, bool, std::less<>>& flags)
    {
      std::ostringstream output;
      bool first = true;
      for (const auto& [flagName, enabled] : flags)
      {
        if (!first)
        {
          output << ", ";
        }
        first = false;
        output << flagName << '=' << (enabled ? "true" : "false");
      }
      return output.str();
    }
  } // namespace

  void DebugOverlay::SetValue(const std::string& key, const std::string& value)
  {
    m_values[key] = value;
  }

  void DebugOverlay::SetRuntimeDescriptor(const Peter::Adapters::RuntimeDescriptor& descriptor)
  {
    SetValue("Runtime Mode", Peter::Adapters::ToString(descriptor.mode));
    SetValue("Runtime Backend", descriptor.backendId);
  }

  void DebugOverlay::SetFeatureFlags(const std::map<std::string, bool, std::less<>>& flags)
  {
    SetValue("Feature Flags", FormatFeatureFlags(flags));
  }

  void DebugOverlay::SetAiSnapshot(const Peter::AI::AgentExplainSnapshot& snapshot)
  {
    SetValue("AI Goal", snapshot.currentGoal);
    SetValue("AI Action", snapshot.lastCompletedAction);
    SetValue("AI Confidence", snapshot.confidenceLabel);
    SetValue("AI Risk", snapshot.riskIndicator);
    SetValue("AI Route", snapshot.routeNodeId);
    SetValue("AI Top Reason", snapshot.topReason);
    SetValue("AI Secondary Reason", snapshot.secondaryReason);
    SetValue("AI Edit Delta", snapshot.editDelta);
    SetValue("AI Alert", snapshot.blackboard.alertLevel);
  }

  void DebugOverlay::SetSaveHealthReport(const Peter::Core::SaveHealthReport& report)
  {
    SetValue("Save Health", report.healthy ? "healthy" : "needs_attention");
    SetValue("Save Domains Checked", std::to_string(report.checkedDomains));
    SetValue("Save Domains Restored", std::to_string(report.restoredDomains));
  }

  void DebugOverlay::SetQualityReport(const Peter::Telemetry::QualityReport& report)
  {
    SetValue("Quality Gate", report.passed ? "pass" : "fail");
    SetValue("Quality Summary", report.summary);
    SetValue("Quality Profile", report.profileId);
    SetValue("Quality Unmeasured", std::to_string(report.unmeasuredSamples));
  }

  std::string DebugOverlay::Render() const
  {
    std::ostringstream output;
    output << "Debug Overlay\n";

    for (const auto& [key, value] : m_values)
    {
      output << "- " << key << ": " << value << '\n';
    }

    return output.str();
  }
} // namespace Peter::Debug
