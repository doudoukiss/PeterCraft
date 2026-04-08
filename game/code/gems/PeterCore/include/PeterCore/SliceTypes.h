#pragma once

#include <map>
#include <string>

namespace Peter::Core
{
  using SliceItemLedger = std::map<std::string, int, std::less<>>;

  struct RaidSessionState
  {
    std::string missionId;
    std::string sceneId;
    std::string currentRoomId;
    int roomsVisited = 0;
    int playerHealth = 100;
    bool highRiskRoomVisited = false;
    bool success = false;
    bool failed = false;
  };

  struct ExtractionResult
  {
    bool success = false;
    std::string reason;
    SliceItemLedger extractedItems;
    SliceItemLedger lostItems;
  };
} // namespace Peter::Core
