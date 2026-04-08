#pragma once

#include "PeterAI/CompanionAi.h"

#include <string>
#include <vector>

namespace Peter::World
{
  struct StationDefinition
  {
    std::string id;
    std::string displayName;
    std::string helpText;
    std::string panelId;
  };

  struct HomeBaseDefinition
  {
    std::string sceneId;
    std::string displayName;
    std::vector<StationDefinition> stations;
  };

  struct RoomDefinition
  {
    std::string id;
    std::string displayName;
    std::string roomType;
    std::string landmarkLabel;
    std::vector<std::string> exits;
    bool optionalPath = false;
    bool highRiskReward = false;
    bool extractionPoint = false;
  };

  struct EncounterDefinition
  {
    std::string id;
    std::string roomId;
    std::vector<Peter::AI::EnemyUnit> enemies;
    bool optional = false;
  };

  struct ExtractionSettings
  {
    std::string id;
    int countdownSeconds = 5;
    std::string successCueId;
    std::string failCueId;
  };

  struct RaidZoneDefinition
  {
    std::string sceneId;
    std::string missionId;
    std::string displayName;
    std::string entryRoomId;
    std::string extractionRoomId;
    std::vector<RoomDefinition> rooms;
    std::vector<EncounterDefinition> encounters;
    ExtractionSettings extraction;
  };

  [[nodiscard]] HomeBaseDefinition BuildPhase1HomeBase();
  [[nodiscard]] RaidZoneDefinition BuildPhase1RaidZone();
} // namespace Peter::World
