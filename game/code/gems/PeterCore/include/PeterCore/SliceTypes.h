#pragma once

#include <map>
#include <string>
#include <vector>

namespace Peter::Core
{
  using SliceItemLedger = std::map<std::string, int, std::less<>>;

  struct RaidSessionState
  {
    int schemaVersion = 2;
    std::string missionId;
    std::string missionTemplateId;
    std::string sceneId;
    std::string currentRoomId;
    int roomsVisited = 0;
    int playerHealth = 100;
    bool highRiskRoomVisited = false;
    bool reducedTimePressure = false;
    bool success = false;
    bool failed = false;
    std::vector<std::string> timeline;
  };

  struct ExtractionResult
  {
    int schemaVersion = 2;
    bool success = false;
    std::string reason;
    SliceItemLedger extractedItems;
    SliceItemLedger lostItems;
    SliceItemLedger brokenItems;
    SliceItemLedger recoveredItems;
  };
} // namespace Peter::Core
