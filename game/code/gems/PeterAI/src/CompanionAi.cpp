#include "PeterAI/CompanionAi.h"

#include <sstream>

namespace Peter::AI
{
  CompanionDecisionSnapshot EvaluateCompanion(
    const CompanionConfig& config,
    const CompanionWorldContext& context)
  {
    CompanionDecisionSnapshot snapshot;

    if (context.playerNeedsRevive && context.repairPulseUnlocked)
    {
      snapshot.currentState = "support";
      snapshot.lastAction = "revive_with_repair_pulse";
      snapshot.topReason = "Player needs immediate help and the repair pulse is ready.";
      snapshot.influentialVariableA = "repair_pulse_unlocked=true";
      snapshot.influentialVariableB = "revive_priority=highest";
    }
    else if (context.playerLowHealth && context.repairPulseUnlocked)
    {
      snapshot.currentState = "support";
      snapshot.lastAction = "repair_pulse";
      snapshot.topReason = "Player health is low, so the companion is stabilizing the run.";
      snapshot.influentialVariableA = "player_health=low";
      snapshot.influentialVariableB = "repair_pulse_unlocked=true";
    }
    else if (context.unsafeToAdvance && context.guardProtocolUnlocked)
    {
      snapshot.currentState = "support_guard";
      snapshot.lastAction = "deploy_guard_field";
      snapshot.topReason = "The area is unsafe, so the companion is buying safer space.";
      snapshot.influentialVariableA = "guard_protocol_unlocked=true";
      snapshot.influentialVariableB = "threat_pressure=high";
    }
    else if (context.threatVisible && context.sameTargetMarked)
    {
      snapshot.currentState = "combat_support";
      snapshot.lastAction = context.guardProtocolUnlocked ? "attack_and_guard" : "attack_same_target";
      snapshot.topReason = "Player marked a nearby threat and the companion can support that target.";
      snapshot.influentialVariableA =
        std::string("follow_distance=") + std::to_string(config.followDistanceMeters);
      snapshot.influentialVariableB =
        std::string("distance_to_player=") + std::to_string(context.distanceToPlayerMeters);
    }
    else if (context.extractionActive)
    {
      snapshot.currentState = "escort";
      snapshot.lastAction = context.timedMissionPressure ? "cover_fast_extraction" : "cover_extraction";
      snapshot.topReason = context.timedMissionPressure
        ? "The mission timer is active, so the companion is prioritizing a quick exit."
        : "Extraction is active and the companion is covering the route.";
      snapshot.influentialVariableA = "extraction_priority=high";
      snapshot.influentialVariableB = context.timedMissionPressure ? "timer_pressure=true" : "timer_pressure=false";
    }
    else if (context.rareLootVisible && context.lootPingUnlocked &&
      context.distanceToPlayerMeters <= static_cast<int>(config.followDistanceMeters))
    {
      snapshot.currentState = "support_loot";
      snapshot.lastAction = "ping_rare_loot";
      snapshot.topReason = "A valuable pickup is nearby and the companion can call it out clearly.";
      snapshot.influentialVariableA = "loot_ping_unlocked=true";
      snapshot.influentialVariableB =
        std::string("follow_distance=") + std::to_string(config.followDistanceMeters);
    }
    else if (context.distanceToPlayerMeters > static_cast<int>(config.followDistanceMeters))
    {
      snapshot.currentState = "regroup";
      snapshot.lastAction = "close_gap";
      snapshot.topReason = "Player is outside the follow distance.";
      snapshot.influentialVariableA =
        std::string("distance_to_player=") + std::to_string(context.distanceToPlayerMeters);
      snapshot.influentialVariableB =
        std::string("follow_distance=") + std::to_string(config.followDistanceMeters);
    }
    else if (config.holdPosition)
    {
      snapshot.currentState = "hold";
      snapshot.lastAction = "hold_position";
      snapshot.topReason = "Player asked the companion to hold position.";
      snapshot.influentialVariableA = "hold_position=true";
      snapshot.influentialVariableB = "threat_visible=false";
    }
    else
    {
      snapshot.currentState = "follow";
      snapshot.lastAction = "follow_player";
      snapshot.topReason = "Stay near the player and be ready to help.";
      snapshot.influentialVariableA =
        std::string("follow_distance=") + std::to_string(config.followDistanceMeters);
      snapshot.influentialVariableB = "support_mode=balanced";
    }

    snapshot.debugSummary = snapshot.currentState + "|" + snapshot.lastAction + "|" + snapshot.topReason;
    return snapshot;
  }

  std::string RenderExplainText(const CompanionDecisionSnapshot& snapshot)
  {
    std::ostringstream output;
    output << "State: " << snapshot.currentState << "\n"
           << "Last action: " << snapshot.lastAction << "\n"
           << "Reason: " << snapshot.topReason << "\n"
           << "Signals: " << snapshot.influentialVariableA << ", " << snapshot.influentialVariableB;
    return output.str();
  }

  Peter::Core::StructuredFields ToSaveFields(const CompanionConfig& config)
  {
    return Peter::Core::StructuredFields{
      {"schema_version", "2"},
      {"follow_distance_meters", std::to_string(config.followDistanceMeters)},
      {"hold_position", config.holdPosition ? "true" : "false"}
    };
  }

  CompanionConfig CompanionConfigFromSaveFields(const Peter::Core::StructuredFields& fields)
  {
    CompanionConfig config;
    const auto distance = fields.find("follow_distance_meters");
    const auto hold = fields.find("hold_position");
    if (distance != fields.end())
    {
      config.followDistanceMeters = std::stod(distance->second);
    }
    config.holdPosition = hold != fields.end() && hold->second == "true";
    return config;
  }

  std::string_view ToString(const EnemyVariant variant)
  {
    switch (variant)
    {
      case EnemyVariant::MeleeChaser:
        return "melee_chaser";
      case EnemyVariant::AlarmSupport:
        return "alarm_support";
    }
    return "unknown";
  }

  EnemyUnit BuildEnemyUnit(
    const std::string_view enemyId,
    const EnemyVariant variant,
    const std::string_view roomId)
  {
    return EnemyUnit{
      std::string(enemyId),
      variant,
      std::string(roomId),
      false,
      variant == EnemyVariant::MeleeChaser ? 24 : 18,
      true};
  }
} // namespace Peter::AI
