#pragma once

#include "PeterAI/CompanionAi.h"

#include <string>
#include <vector>

namespace Peter::Combat
{
  struct EncounterRequest
  {
    std::vector<Peter::AI::EnemyUnit> enemies;
    Peter::AI::CompanionDecisionSnapshot companionDecision;
    bool playerChoosesStealth = false;
    bool highRiskRoom = false;
    int playerHealth = 100;
  };

  struct EncounterOutcome
  {
    bool playerDefeated = false;
    bool alarmTriggered = false;
    int playerDamage = 0;
    int enemiesDefeated = 0;
    std::string causeOfFailure;
    std::string summary;
  };

  [[nodiscard]] EncounterOutcome ResolveEncounter(const EncounterRequest& request);
} // namespace Peter::Combat
