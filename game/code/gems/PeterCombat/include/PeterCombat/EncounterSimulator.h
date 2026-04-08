#pragma once

#include "PeterAI/CompanionAi.h"
#include "PeterCore/EventBus.h"

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace Peter::Combat
{
  enum class CombatActionKind
  {
    DirectHit,
    AreaEffect,
    SupportAction,
    StatusTick
  };

  struct StatusEffectSpec
  {
    std::string effectId;
    int durationTurns = 1;
    int potency = 0;
  };

  struct DamageSpec
  {
    std::string actionId;
    std::string sourceId;
    std::string targetId;
    CombatActionKind kind = CombatActionKind::DirectHit;
    int baseAmount = 0;
    std::vector<std::string> tags;
    std::vector<StatusEffectSpec> appliedStatuses;
  };

  struct SupportActionSpec
  {
    std::string actionId;
    std::string sourceId;
    std::string targetId;
    int healAmount = 0;
    int invulnerabilityTurns = 0;
    std::vector<std::string> tags;
    std::vector<StatusEffectSpec> grantedStatuses;
  };

  struct CombatantState
  {
    std::string combatantId;
    int health = 100;
    int maxHealth = 100;
    int invulnerabilityTurns = 0;
    std::map<std::string, int, std::less<>> activeStatusDuration;
    std::map<std::string, int, std::less<>> activeStatusPotency;
  };

  struct DamageOutcome
  {
    std::string actionId;
    std::string sourceId;
    std::string targetId;
    CombatActionKind kind = CombatActionKind::DirectHit;
    int healthBefore = 0;
    int healthAfter = 0;
    int damageApplied = 0;
    int healingApplied = 0;
    bool blockedByInvulnerability = false;
    std::vector<std::string> statusesApplied;
    std::vector<std::string> tags;
    std::string explanation;
  };

  struct EncounterRequest
  {
    std::vector<Peter::AI::EnemyUnit> enemies;
    Peter::AI::CompanionDecisionSnapshot companionDecision;
    bool playerChoosesStealth = false;
    bool highRiskRoom = false;
    bool timedMissionPressure = false;
    int playerHealth = 100;
  };

  struct EncounterOutcome
  {
    bool playerDefeated = false;
    bool alarmTriggered = false;
    int playerDamage = 0;
    int playerHealing = 0;
    int enemiesDefeated = 0;
    std::string causeOfFailure;
    std::string summary;
    std::string companionHighlight;
    std::vector<DamageOutcome> actionOutcomes;
    std::vector<Peter::Core::Event> events;
  };

  [[nodiscard]] std::string_view ToString(CombatActionKind kind);
  [[nodiscard]] DamageOutcome ResolveDamageAction(CombatantState& target, const DamageSpec& spec);
  [[nodiscard]] DamageOutcome ResolveSupportAction(CombatantState& target, const SupportActionSpec& spec);
  [[nodiscard]] std::vector<DamageOutcome> TickStatuses(CombatantState& target, std::string_view sourceId);
  [[nodiscard]] EncounterOutcome ResolveEncounter(const EncounterRequest& request);
} // namespace Peter::Combat
