#pragma once

#include "PeterAI/CompanionAi.h"
#include "PeterWorld/SliceContent.h"

#include <string>

namespace Peter::UI
{
  struct PostRaidSummaryModel
  {
    bool success = false;
    std::string extractedItems;
    std::string lostItems;
    std::string companionHighlight;
    std::string nextHint;
  };

  [[nodiscard]] std::string RenderHomeBaseOverview(const Peter::World::HomeBaseDefinition& homeBase);
  [[nodiscard]] std::string RenderRaidZoneOverview(const Peter::World::RaidZoneDefinition& raidZone);
  [[nodiscard]] std::string RenderCompanionExplainPanel(const Peter::AI::CompanionDecisionSnapshot& snapshot);
  [[nodiscard]] std::string RenderPostRaidSummary(const PostRaidSummaryModel& model);
} // namespace Peter::UI
