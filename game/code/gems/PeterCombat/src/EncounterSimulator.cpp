#include "PeterCombat/EncounterSimulator.h"

#include <algorithm>
#include <sstream>

namespace Peter::Combat
{
  namespace
  {
    int PotencyFor(const CombatantState& target, std::string_view statusId)
    {
      const auto iterator = target.activeStatusPotency.find(std::string(statusId));
      return iterator == target.activeStatusPotency.end() ? 0 : iterator->second;
    }

    void ApplyStatus(CombatantState& target, const StatusEffectSpec& status)
    {
      target.activeStatusDuration[status.effectId] = status.durationTurns;
      target.activeStatusPotency[status.effectId] = status.potency;
    }

    Peter::Core::Event BuildCombatEvent(const DamageOutcome& outcome)
    {
      std::ostringstream tags;
      bool firstTag = true;
      for (const auto& tag : outcome.tags)
      {
        if (!firstTag)
        {
          tags << ",";
        }
        tags << tag;
        firstTag = false;
      }

      std::ostringstream statuses;
      bool firstStatus = true;
      for (const auto& status : outcome.statusesApplied)
      {
        if (!firstStatus)
        {
          statuses << ",";
        }
        statuses << status;
        firstStatus = false;
      }

      return Peter::Core::Event{
        Peter::Core::EventCategory::Gameplay,
        "gameplay.combat.action",
        {
          {"action_id", outcome.actionId},
          {"blocked", outcome.blockedByInvulnerability ? "true" : "false"},
          {"damage_applied", std::to_string(outcome.damageApplied)},
          {"explanation", outcome.explanation},
          {"healing_applied", std::to_string(outcome.healingApplied)},
          {"health_after", std::to_string(outcome.healthAfter)},
          {"health_before", std::to_string(outcome.healthBefore)},
          {"kind", std::string(ToString(outcome.kind))},
          {"source_id", outcome.sourceId},
          {"statuses", statuses.str()},
          {"tags", tags.str()},
          {"target_id", outcome.targetId}
        }};
    }

    void AppendOutcome(EncounterOutcome& encounter, const DamageOutcome& outcome)
    {
      encounter.actionOutcomes.push_back(outcome);
      encounter.events.push_back(BuildCombatEvent(outcome));
      encounter.playerDamage += outcome.damageApplied;
      encounter.playerHealing += outcome.healingApplied;
    }
  } // namespace

  std::string_view ToString(const CombatActionKind kind)
  {
    switch (kind)
    {
      case CombatActionKind::DirectHit:
        return "direct_hit";
      case CombatActionKind::AreaEffect:
        return "area_effect";
      case CombatActionKind::SupportAction:
        return "support_action";
      case CombatActionKind::StatusTick:
        return "status_tick";
    }

    return "direct_hit";
  }

  DamageOutcome ResolveDamageAction(CombatantState& target, const DamageSpec& spec)
  {
    DamageOutcome outcome;
    outcome.actionId = spec.actionId;
    outcome.sourceId = spec.sourceId;
    outcome.targetId = spec.targetId;
    outcome.kind = spec.kind;
    outcome.healthBefore = target.health;
    outcome.tags = spec.tags;

    if (target.invulnerabilityTurns > 0)
    {
      outcome.blockedByInvulnerability = true;
      outcome.healthAfter = target.health;
      outcome.explanation = "Target was in an invulnerability window.";
      return outcome;
    }

    int effectiveDamage = spec.baseAmount;
    effectiveDamage = std::max(0, effectiveDamage - PotencyFor(target, "guarded"));
    effectiveDamage += PotencyFor(target, "staggered");
    target.health = std::max(0, target.health - effectiveDamage);

    outcome.damageApplied = effectiveDamage;
    outcome.healthAfter = target.health;

    for (const auto& status : spec.appliedStatuses)
    {
      ApplyStatus(target, status);
      outcome.statusesApplied.push_back(status.effectId);
    }

    std::ostringstream explanation;
    explanation << "Applied " << effectiveDamage << " damage from " << spec.actionId;
    if (!outcome.statusesApplied.empty())
    {
      explanation << " and applied " << outcome.statusesApplied.front();
    }
    outcome.explanation = explanation.str();
    return outcome;
  }

  DamageOutcome ResolveSupportAction(CombatantState& target, const SupportActionSpec& spec)
  {
    DamageOutcome outcome;
    outcome.actionId = spec.actionId;
    outcome.sourceId = spec.sourceId;
    outcome.targetId = spec.targetId;
    outcome.kind = CombatActionKind::SupportAction;
    outcome.healthBefore = target.health;
    outcome.tags = spec.tags;

    const int healed = std::min(spec.healAmount, target.maxHealth - target.health);
    target.health += healed;
    outcome.healingApplied = healed;

    for (const auto& status : spec.grantedStatuses)
    {
      ApplyStatus(target, status);
      outcome.statusesApplied.push_back(status.effectId);
    }

    if (spec.invulnerabilityTurns > 0)
    {
      target.invulnerabilityTurns = spec.invulnerabilityTurns;
    }

    outcome.healthAfter = target.health;
    std::ostringstream explanation;
    explanation << "Support action " << spec.actionId << " restored " << healed << " health";
    if (target.invulnerabilityTurns > 0)
    {
      explanation << " and granted an invulnerability window";
    }
    outcome.explanation = explanation.str();
    return outcome;
  }

  std::vector<DamageOutcome> TickStatuses(CombatantState& target, const std::string_view sourceId)
  {
    std::vector<DamageOutcome> outcomes;
    std::vector<std::string> expired;

    for (const auto& [statusId, duration] : target.activeStatusDuration)
    {
      const int potency = PotencyFor(target, statusId);
      if (statusId == "overheat")
      {
        DamageOutcome outcome;
        outcome.actionId = "status.overheat.tick";
        outcome.sourceId = std::string(sourceId);
        outcome.targetId = target.combatantId;
        outcome.kind = CombatActionKind::StatusTick;
        outcome.healthBefore = target.health;
        outcome.damageApplied = potency;
        outcome.healthAfter = std::max(0, target.health - potency);
        target.health = outcome.healthAfter;
        outcome.statusesApplied = {statusId};
        outcome.tags = {"status", "heat"};
        outcome.explanation = "Overheat dealt tick damage.";
        outcomes.push_back(outcome);
      }
      else if (statusId == "repair_boost")
      {
        DamageOutcome outcome;
        outcome.actionId = "status.repair_boost.tick";
        outcome.sourceId = std::string(sourceId);
        outcome.targetId = target.combatantId;
        outcome.kind = CombatActionKind::StatusTick;
        outcome.healthBefore = target.health;
        const int healed = std::min(potency, target.maxHealth - target.health);
        target.health += healed;
        outcome.healingApplied = healed;
        outcome.healthAfter = target.health;
        outcome.statusesApplied = {statusId};
        outcome.tags = {"status", "support"};
        outcome.explanation = "Repair boost restored health over time.";
        outcomes.push_back(outcome);
      }

      if (duration <= 1)
      {
        expired.push_back(statusId);
      }
      else
      {
        target.activeStatusDuration[statusId] = duration - 1;
      }
    }

    for (const auto& statusId : expired)
    {
      target.activeStatusDuration.erase(statusId);
      target.activeStatusPotency.erase(statusId);
    }

    if (target.invulnerabilityTurns > 0)
    {
      --target.invulnerabilityTurns;
    }

    return outcomes;
  }

  EncounterOutcome ResolveEncounter(const EncounterRequest& request)
  {
    EncounterOutcome outcome;
    CombatantState player{"player", request.playerHealth, 100};

    if (request.companionDecision.lastAction == "deploy_guard_field")
    {
      AppendOutcome(
        outcome,
        ResolveSupportAction(
          player,
          SupportActionSpec{
            "support.guard_field",
            "companion",
            "player",
            0,
            0,
            {"support", "guard"},
            {StatusEffectSpec{"guarded", 1, 6}}}));
      outcome.companionHighlight = "Companion deployed a guard field.";
    }
    else if (request.companionDecision.lastAction == "repair_pulse" ||
      request.companionDecision.lastAction == "revive_with_repair_pulse")
    {
      AppendOutcome(
        outcome,
        ResolveSupportAction(
          player,
          SupportActionSpec{
            request.companionDecision.lastAction == "revive_with_repair_pulse"
              ? "support.revive_with_repair_pulse"
              : "support.repair_pulse",
            "companion",
            "player",
            request.companionDecision.lastAction == "revive_with_repair_pulse" ? 16 : 10,
            request.companionDecision.lastAction == "revive_with_repair_pulse" ? 1 : 0,
            {"support", "repair"},
            {StatusEffectSpec{"repair_boost", 1, 4}}}));
      outcome.companionHighlight = "Companion stabilized the player with a repair pulse.";
    }

    for (const auto& enemy : request.enemies)
    {
      if (!enemy.active)
      {
        continue;
      }

      if (enemy.variant == Peter::AI::EnemyVariant::MeleeChaser)
      {
        AppendOutcome(
          outcome,
          ResolveDamageAction(
            player,
            DamageSpec{
              "enemy.chaser.direct_hit",
              enemy.enemyId,
              "player",
              CombatActionKind::DirectHit,
              request.highRiskRoom ? 16 : 12,
              {"machine", "kinetic"},
              {StatusEffectSpec{"staggered", 1, 2}}}));
      }
      else
      {
        const bool alarmMode = request.playerChoosesStealth || request.highRiskRoom || request.timedMissionPressure;
        outcome.alarmTriggered = outcome.alarmTriggered || alarmMode;
        AppendOutcome(
          outcome,
          ResolveDamageAction(
            player,
            DamageSpec{
              alarmMode ? "enemy.alarm_support.shockwave" : "enemy.alarm_support.burst",
              enemy.enemyId,
              "player",
              alarmMode ? CombatActionKind::AreaEffect : CombatActionKind::DirectHit,
              alarmMode ? 10 : 7,
              alarmMode ? std::vector<std::string>{"machine", "area", "alarm"} : std::vector<std::string>{"machine", "support"},
              alarmMode ? std::vector<StatusEffectSpec>{StatusEffectSpec{"overheat", 1, 3}}
                        : std::vector<StatusEffectSpec>{StatusEffectSpec{"staggered", 1, 1}}}));
      }
    }

    for (const auto& statusOutcome : TickStatuses(player, "system.status"))
    {
      AppendOutcome(outcome, statusOutcome);
    }

    if (request.companionDecision.currentState == "combat_support" ||
      request.companionDecision.currentState == "escort" ||
      request.companionDecision.currentState == "support_guard")
    {
      outcome.enemiesDefeated = static_cast<int>(request.enemies.size());
    }
    else
    {
      outcome.enemiesDefeated = std::max(1, static_cast<int>(request.enemies.size()) - 1);
    }

    outcome.playerDefeated = player.health <= 0;
    if (outcome.playerDefeated)
    {
      outcome.causeOfFailure = outcome.alarmTriggered
        ? "alarm_wave_overwhelmed_player"
        : "combat_damage_overwhelmed_player";
    }

    std::ostringstream summary;
    summary << "Combat resolved with " << outcome.actionOutcomes.size() << " logged actions"
            << ", damage=" << outcome.playerDamage
            << ", healing=" << outcome.playerHealing
            << ", remaining_health=" << player.health;
    outcome.summary = summary.str();

    if (outcome.companionHighlight.empty())
    {
      outcome.companionHighlight = request.companionDecision.topReason;
    }

    return outcome;
  }
} // namespace Peter::Combat
