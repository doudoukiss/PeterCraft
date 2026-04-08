#include "PeterUI/SlicePresentation.h"

#include <sstream>

namespace Peter::UI
{
  std::string RenderHomeBaseOverview(const Peter::World::HomeBaseDefinition& homeBase)
  {
    std::ostringstream output;
    output << homeBase.displayName << ": ";
    bool first = true;
    for (const auto& station : homeBase.stations)
    {
      if (!first)
      {
        output << " | ";
      }
      output << station.displayName;
      first = false;
    }
    return output.str();
  }

  std::string RenderRaidZoneOverview(const Peter::World::RaidZoneDefinition& raidZone)
  {
    std::ostringstream output;
    output << raidZone.displayName << " rooms=" << raidZone.rooms.size()
           << " extraction=" << raidZone.extraction.countdownSeconds << "s";
    return output.str();
  }

  std::string RenderCompanionExplainPanel(const Peter::AI::CompanionDecisionSnapshot& snapshot)
  {
    return Peter::AI::RenderExplainText(snapshot);
  }

  std::string RenderPostRaidSummary(const PostRaidSummaryModel& model)
  {
    std::ostringstream output;
    output << (model.success ? "Raid success" : "Raid failed") << "\n"
           << "Extracted: " << model.extractedItems << "\n"
           << "Lost: " << model.lostItems << "\n"
           << "Companion: " << model.companionHighlight << "\n"
           << "Next: " << model.nextHint;
    return output.str();
  }
} // namespace Peter::UI
