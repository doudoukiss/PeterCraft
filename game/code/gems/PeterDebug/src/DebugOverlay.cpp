#include "PeterDebug/DebugOverlay.h"

#include <sstream>

namespace Peter::Debug
{
  void DebugOverlay::SetValue(const std::string& key, const std::string& value)
  {
    m_values[key] = value;
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
