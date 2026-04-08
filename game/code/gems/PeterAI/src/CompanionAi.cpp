#include "PeterAI/CompanionAi.h"

#include <sstream>

namespace Peter::AI
{
  CompanionDecisionSnapshot EvaluateCompanion(
    const CompanionConfig& config,
    const CompanionWorldContext& context)
  {
    CompanionDecisionSnapshot snapshot;

    if (context.playerNeedsRevive)
    {
      snapshot.currentState = "support";
      snapshot.lastAction = "revive_player";
      snapshot.topReason = "Player needs help immediately.";
      snapshot.influentialVariableA = "revive_priority=highest";
      snapshot.influentialVariableB = "player_health=critical";
    }
    else if (context.unsafeToAdvance)
    {
      snapshot.currentState = "retreat";
      snapshot.lastAction = "retreat_to_cover";
      snapshot.topReason = "Area is unsafe for the companion.";
      snapshot.influentialVariableA = "threat_pressure=high";
      snapshot.influentialVariableB = "safety_override=true";
    }
    else if (context.threatVisible && context.sameTargetMarked)
    {
      snapshot.currentState = "combat_support";
      snapshot.lastAction = "attack_same_target";
      snapshot.topReason = "Player marked a nearby threat.";
      snapshot.influentialVariableA =
        std::string("follow_distance=") + std::to_string(config.followDistanceMeters);
      snapshot.influentialVariableB =
        std::string("distance_to_player=") + std::to_string(context.distanceToPlayerMeters);
    }
    else if (context.extractionActive)
    {
      snapshot.currentState = "escort";
      snapshot.lastAction = "cover_extraction";
      snapshot.topReason = "Extraction is active.";
      snapshot.influentialVariableA = "extraction_priority=high";
      snapshot.influentialVariableB = "hold_position=false";
    }
    else if (context.rareLootVisible && context.distanceToPlayerMeters <= static_cast<int>(config.followDistanceMeters))
    {
      snapshot.currentState = "support_loot";
      snapshot.lastAction = "mark_rare_loot";
      snapshot.topReason = "Interesting salvage is nearby and the player is still within range.";
      snapshot.influentialVariableA = "rare_loot_visible=true";
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
    return EnemyUnit{std::string(enemyId), variant, std::string(roomId), false, 20, true};
  }
} // namespace Peter::AI
