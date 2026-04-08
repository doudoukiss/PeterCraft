#include "PeterCombat/EncounterSimulator.h"

#include <sstream>

namespace Peter::Combat
{
  EncounterOutcome ResolveEncounter(const EncounterRequest& request)
  {
    EncounterOutcome outcome;
    int totalThreat = 0;
    bool hasAlarmSupport = false;

    for (const auto& enemy : request.enemies)
    {
      totalThreat += enemy.variant == Peter::AI::EnemyVariant::MeleeChaser ? 20 : 15;
      hasAlarmSupport = hasAlarmSupport || enemy.variant == Peter::AI::EnemyVariant::AlarmSupport;
    }

    const bool companionInTime = request.companionDecision.currentState == "combat_support" ||
      request.companionDecision.currentState == "escort";
    const bool companionDelayed = request.companionDecision.currentState == "regroup";

    if (request.playerChoosesStealth && hasAlarmSupport)
    {
      outcome.alarmTriggered = true;
      totalThreat += 10;
    }

    if (companionInTime)
    {
      totalThreat -= 20;
    }
    else if (companionDelayed)
    {
      totalThreat += 10;
    }

    if (request.highRiskRoom)
    {
      totalThreat += 10;
    }

    outcome.playerDamage = totalThreat / 4;
    outcome.enemiesDefeated = static_cast<int>(request.enemies.size());
    outcome.playerDefeated = request.playerHealth - outcome.playerDamage <= 0;
    outcome.causeOfFailure = outcome.playerDefeated ? "machine_patrol_overwhelmed_player" : "";

    std::ostringstream output;
    output << "Encounter resolved with " << outcome.enemiesDefeated << " enemies defeated";
    if (outcome.alarmTriggered)
    {
      output << ", alarm triggered";
    }
    output << ", damage taken=" << outcome.playerDamage;
    outcome.summary = output.str();

    return outcome;
  }
} // namespace Peter::Combat
