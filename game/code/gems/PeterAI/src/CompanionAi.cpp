#include "PeterAI/CompanionAi.h"

#include <algorithm>
#include <iomanip>
#include <set>
#include <sstream>
#include <utility>

namespace Peter::AI
{
  namespace
  {
    constexpr std::string_view kDefaultStanceId = "stance.balanced";
    constexpr std::string_view kStayNearChipId = "chip.stay_near_me";

    std::string SerializeStrings(const std::vector<std::string>& values)
    {
      std::ostringstream output;
      bool first = true;
      for (const auto& value : values)
      {
        if (!first)
        {
          output << ",";
        }
        output << value;
        first = false;
      }
      return output.str();
    }

    std::vector<std::string> SplitStrings(const std::string_view serialized)
    {
      std::vector<std::string> values;
      std::size_t cursor = 0;
      while (cursor < serialized.size())
      {
        const auto separator = serialized.find(',', cursor);
        const auto token = serialized.substr(
          cursor,
          separator == std::string_view::npos ? serialized.size() - cursor : separator - cursor);
        if (!token.empty())
        {
          values.emplace_back(token);
        }
        if (separator == std::string_view::npos)
        {
          break;
        }
        cursor = separator + 1;
      }
      return values;
    }

    std::string SerializeChipValues(const std::map<std::string, double, std::less<>>& values)
    {
      std::ostringstream output;
      bool first = true;
      for (const auto& [chipId, value] : values)
      {
        if (!first)
        {
          output << ",";
        }
        output << chipId << "=" << std::fixed << std::setprecision(2) << value;
        first = false;
      }
      return output.str();
    }

    std::map<std::string, double, std::less<>> ParseChipValues(const std::string_view serialized)
    {
      std::map<std::string, double, std::less<>> values;
      for (const auto& token : SplitStrings(serialized))
      {
        const auto equals = token.find('=');
        if (equals == std::string::npos)
        {
          continue;
        }
        values[token.substr(0, equals)] = std::stod(token.substr(equals + 1));
      }
      return values;
    }

    template <typename T>
    bool Contains(const std::vector<T>& values, const T& target)
    {
      return std::find(values.begin(), values.end(), target) != values.end();
    }

    const BehaviorStanceDefinition& SafeStance(std::string_view stanceId)
    {
      const auto* stance = FindBehaviorStance(stanceId);
      return stance == nullptr ? BuildPhase3Stances().at(1) : *stance;
    }

    double ChipValue(const CompanionConfig& config, const std::string_view chipId)
    {
      const auto iterator = config.chipValues.find(std::string(chipId));
      if (iterator != config.chipValues.end())
      {
        return iterator->second;
      }

      const auto* definition = FindBehaviorChip(chipId);
      return definition == nullptr ? 0.0 : definition->defaultValue;
    }

    std::vector<std::string> NormalizeChipIds(const CompanionConfig& config)
    {
      std::vector<std::string> normalized;
      std::set<std::string, std::less<>> unique;

      for (const auto& chipId : config.activeChipIds)
      {
        if (chipId.empty() || FindBehaviorChip(chipId) == nullptr)
        {
          continue;
        }
        if (unique.insert(chipId).second)
        {
          normalized.push_back(chipId);
        }
        if (normalized.size() == 3)
        {
          break;
        }
      }

      if (normalized.empty())
      {
        normalized.push_back(std::string(kStayNearChipId));
      }
      else if (!Contains(normalized, std::string(kStayNearChipId)) && normalized.size() < 3)
      {
        normalized.insert(normalized.begin(), std::string(kStayNearChipId));
      }

      return normalized;
    }

    CompanionConfig NormalizeConfig(CompanionConfig config)
    {
      config.activeChipIds = NormalizeChipIds(config);
      config.followDistanceMeters = ResolveFollowDistance(config);
      if (FindBehaviorStance(config.stanceId) == nullptr)
      {
        config.stanceId = std::string(kDefaultStanceId);
      }
      return config;
    }

    int ActionPriority(const std::string_view actionId)
    {
      if (actionId == "revive")
      {
        return 0;
      }
      if (actionId == "extract")
      {
        return 1;
      }
      if (actionId == "evade")
      {
        return 2;
      }
      if (actionId == "attack")
      {
        return 3;
      }
      if (actionId == "loot")
      {
        return 4;
      }
      if (actionId == "interact")
      {
        return 5;
      }
      if (actionId == "move_to")
      {
        return 6;
      }
      if (actionId == "hold")
      {
        return 7;
      }
      return 8;
    }

    DecisionCandidate MakeCandidate(std::string actionId, std::string goalId)
    {
      const auto priority = ActionPriority(actionId);
      return DecisionCandidate{std::move(actionId), std::move(goalId), 0.0, priority, {}};
    }

    void AddReason(
      DecisionCandidate& candidate,
      const std::string_view label,
      const double delta,
      const std::string_view explanation)
    {
      candidate.score += delta;
      candidate.breakdowns.push_back(DecisionBreakdown{std::string(label), delta, std::string(explanation)});
    }

    std::string JoinTopReasons(const std::vector<DecisionCandidate>& candidates)
    {
      std::ostringstream output;
      bool first = true;
      for (std::size_t index = 0; index < candidates.size() && index < 3; ++index)
      {
        if (!first)
        {
          output << " | ";
        }
        output << candidates[index].actionId << "=" << std::fixed << std::setprecision(1) << candidates[index].score;
        first = false;
      }
      return output.str();
    }

    std::string BestReasonText(const DecisionCandidate& candidate)
    {
      if (candidate.breakdowns.empty())
      {
        return "No weighted reasons were recorded.";
      }
      const auto best = std::max_element(
        candidate.breakdowns.begin(),
        candidate.breakdowns.end(),
        [](const DecisionBreakdown& left, const DecisionBreakdown& right) {
          return left.delta < right.delta;
        });
      return best == candidate.breakdowns.end() ? "No weighted reasons were recorded." : best->explanation;
    }

    std::string SecondaryReasonText(const DecisionCandidate& candidate)
    {
      if (candidate.breakdowns.size() < 2)
      {
        return "No secondary reason was needed.";
      }
      auto sorted = candidate.breakdowns;
      std::sort(
        sorted.begin(),
        sorted.end(),
        [](const DecisionBreakdown& left, const DecisionBreakdown& right) {
          if (left.delta == right.delta)
          {
            return left.label < right.label;
          }
          return left.delta > right.delta;
        });
      return sorted.at(1).explanation;
    }
    std::string ConfidenceLabel(const std::vector<DecisionCandidate>& candidates)
    {
      if (candidates.empty())
      {
        return "low";
      }
      if (candidates.size() == 1)
      {
        return "high";
      }
      const auto gap = candidates[0].score - candidates[1].score;
      if (gap >= 18.0)
      {
        return "high";
      }
      if (gap >= 8.0)
      {
        return "medium";
      }
      return "low";
    }

    std::string RiskIndicator(const CompanionWorldContext& context)
    {
      if (context.playerNeedsRevive || (context.threatVisible && context.unsafeToAdvance))
      {
        return "high";
      }
      if (context.threatVisible || context.extractionActive || context.playerLowHealth)
      {
        return "medium";
      }
      return "low";
    }

    std::string GoalForAction(const std::string_view actionId, const CompanionWorldContext& context)
    {
      if (actionId == "revive")
      {
        return "goal.revive_player";
      }
      if (actionId == "extract")
      {
        return "goal.reach_extraction";
      }
      if (actionId == "evade")
      {
        return "goal.regroup_safely";
      }
      if (actionId == "attack")
      {
        return "goal.defend_player";
      }
      if (actionId == "loot")
      {
        return "goal.recover_rare_loot";
      }
      if (actionId == "interact")
      {
        return context.currentGoal.empty() ? "goal.interact" : context.currentGoal;
      }
      if (actionId == "move_to")
      {
        return context.currentGoal.empty() ? "goal.follow_player" : context.currentGoal;
      }
      if (actionId == "hold")
      {
        return "goal.hold_position";
      }
      return "goal.wait";
    }

    std::string CompanionStateForAction(
      const std::string_view actionId,
      const CompanionWorldContext& context,
      const CompanionConfig& config)
    {
      (void)config;
      if (actionId == "revive")
      {
        return "revive_help";
      }
      if (actionId == "extract")
      {
        return "extract_assist";
      }
      if (actionId == "evade")
      {
        return "retreat_regroup";
      }
      if (actionId == "attack")
      {
        return "defend_player";
      }
      if (actionId == "loot")
      {
        return "loot_support";
      }
      if (actionId == "hold")
      {
        return "hold_position";
      }
      if (actionId == "move_to" && context.currentGoal == "goal.scout_path")
      {
        return "scout";
      }
      if (actionId == "interact")
      {
        return "scout";
      }
      return "follow";
    }

    std::pair<std::string, std::string> FeedbackTokens(const std::string_view actionId)
    {
      if (actionId == "revive")
      {
        return {"callout.companion.hold_on", "gesture.repair_pulse"};
      }
      if (actionId == "extract")
      {
        return {"callout.companion.to_exit", "gesture.point_exit"};
      }
      if (actionId == "evade")
      {
        return {"callout.companion.fall_back", "gesture.retreat"};
      }
      if (actionId == "attack")
      {
        return {"callout.companion.covering_you", "gesture.mark_threat"};
      }
      if (actionId == "loot")
      {
        return {"callout.companion.rare_loot", "gesture.point_loot"};
      }
      if (actionId == "hold")
      {
        return {"callout.companion.holding_here", "gesture.stop"};
      }
      return {"callout.companion.on_you", "gesture.follow"};
    }

    std::vector<std::string> StanceModifierText(const BehaviorStanceDefinition& stance)
    {
      std::vector<std::string> modifiers;
      if (stance.attackBias > 0.0)
      {
        modifiers.push_back(stance.displayName + " raises attack confidence.");
      }
      if (stance.defendBias > 0.0)
      {
        modifiers.push_back(stance.displayName + " keeps defense near the top.");
      }
      if (stance.lootBias > 0.0)
      {
        modifiers.push_back(stance.displayName + " makes optional loot more tempting.");
      }
      if (stance.cautionBias > 0.0)
      {
        modifiers.push_back(stance.displayName + " pulls the companion toward safer regroups.");
      }
      if (modifiers.empty())
      {
        modifiers.push_back(stance.displayName + " keeps decisions even and readable.");
      }
      return modifiers;
    }

    std::vector<std::string> ChipModifierText(const CompanionConfig& config)
    {
      std::vector<std::string> modifiers;
      for (const auto& chipId : NormalizeChipIds(config))
      {
        const auto* chip = FindBehaviorChip(chipId);
        if (chip == nullptr)
        {
          continue;
        }
        std::ostringstream summary;
        summary << chip->displayName << ": " << chip->promiseText;
        if (chipId == kStayNearChipId)
        {
          summary << " (" << ResolveFollowDistance(config) << "m)";
        }
        modifiers.push_back(summary.str());
      }
      return modifiers;
    }

    std::string EditDelta(const CompanionConfig& config)
    {
      const auto& stance = SafeStance(config.stanceId);
      std::ostringstream output;
      output << stance.displayName << " stance";
      const auto chipIds = NormalizeChipIds(config);
      if (!chipIds.empty())
      {
        output << " with ";
        bool first = true;
        for (const auto& chipId : chipIds)
        {
          const auto* chip = FindBehaviorChip(chipId);
          if (chip == nullptr)
          {
            continue;
          }
          if (!first)
          {
            output << ", ";
          }
          output << chip->displayName;
          first = false;
        }
      }
      return output.str();
    }

    void SortCandidates(std::vector<DecisionCandidate>& candidates)
    {
      std::sort(
        candidates.begin(),
        candidates.end(),
        [](const DecisionCandidate& left, const DecisionCandidate& right) {
          if (left.score == right.score)
          {
            if (left.priority == right.priority)
            {
              return left.actionId < right.actionId;
            }
            return left.priority < right.priority;
          }
          return left.score > right.score;
        });
      if (candidates.size() > 5)
      {
        candidates.resize(5);
      }
    }

    void ApplyOverrides(
      std::vector<DecisionCandidate>& candidates,
      const BehaviorOverrideSet& overrides)
    {
      for (const auto& entry : overrides.overrides)
      {
        auto iterator = std::find_if(
          candidates.begin(),
          candidates.end(),
          [&entry](const DecisionCandidate& candidate) {
            return candidate.actionId == entry.actionId;
          });
        if (iterator == candidates.end())
        {
          continue;
        }

        const double boundedDelta = std::clamp(entry.scoreDelta, -20.0, 20.0);
        if (entry.gateAction)
        {
          AddReason(
            *iterator,
            entry.sourceId.empty() ? "creator_gate" : entry.sourceId,
            -100.0,
            entry.explanation.empty() ? "Creator logic blocked this action for safety." : entry.explanation);
        }

        if (boundedDelta != 0.0)
        {
          AddReason(
            *iterator,
            entry.sourceId.empty() ? "creator_hint" : entry.sourceId,
            boundedDelta,
            entry.explanation.empty() ? "Creator logic adjusted this action." : entry.explanation);
        }
      }
    }

    AgentBlackboardState BuildBlackboard(
      const CompanionConfig& config,
      const CompanionWorldContext& context,
      const PerceptionSnapshot& perception)
    {
      AgentBlackboardState blackboard;
      blackboard.currentTargetId = context.threatVisible ? context.visibleThreatId : context.currentTargetId;
      blackboard.lastKnownThreatPositionToken = context.lastKnownThreatPositionToken;
      blackboard.currentGoalId = context.currentGoal.empty() ? "goal.follow_player" : context.currentGoal;
      blackboard.healthConfidence = std::clamp(context.companionHealthPercent / 100.0, 0.0, 1.0);
      blackboard.playerState = context.playerNeedsRevive
        ? "needs_revive"
        : (context.playerLowHealth ? "low_health" : "stable");
      blackboard.activeStanceId = config.stanceId;
      blackboard.activeChipIds = NormalizeChipIds(config);
      blackboard.extractionUrgency = context.extractionActive
        ? std::max(1, context.extractionUrgency + (context.timedMissionPressure ? 1 : 0))
        : context.extractionUrgency;
      blackboard.lastFailedActionId = context.lastFailedAction;
      blackboard.routeNodeId = context.routeNodeId;
      blackboard.roomNodeId = context.roomNodeId;
      blackboard.alertLevel = std::string(ToString(perception.alertLevel));
      return blackboard;
    }

    std::vector<DecisionCandidate> BuildCompanionCandidates(
      const CompanionConfig& config,
      const CompanionWorldContext& context,
      const PerceptionSnapshot& perception,
      const AgentBlackboardState& blackboard)
    {
      const auto& stance = SafeStance(config.stanceId);
      const auto chipIds = NormalizeChipIds(config);
      const bool protectMeFirst = Contains(chipIds, std::string("chip.protect_me_first"));
      const bool grabRareLoot = Contains(chipIds, std::string("chip.grab_rare_loot"));
      const bool retreatWhenHurt = Contains(chipIds, std::string("chip.retreat_when_hurt"));
      const bool helpAtExtraction = Contains(chipIds, std::string("chip.help_at_extraction"));
      const double followDistance = ResolveFollowDistance(config);

      std::vector<DecisionCandidate> candidates;

      auto revive = MakeCandidate("revive", "goal.revive_player");
      if (context.playerNeedsRevive)
      {
        AddReason(revive, "player_needs_revive", 80.0, "Player needs help right now.");
      }
      if (context.repairPulseUnlocked)
      {
        AddReason(revive, "repair_pulse", 12.0, "Repair Pulse is ready for a safe revive.");
      }
      if (protectMeFirst)
      {
        AddReason(revive, "protect_me_first", 8.0, "Protect Me First keeps rescue actions high.");
      }
      if (stance.reviveBias != 0.0)
      {
        AddReason(revive, "stance_revive_bias", stance.reviveBias * 6.0, "The active stance leans toward recovery support.");
      }
      if (!context.playerNeedsRevive)
      {
        AddReason(revive, "no_revive_target", -30.0, "No one needs a revive right now.");
      }
      candidates.push_back(revive);

      auto extract = MakeCandidate("extract", "goal.reach_extraction");
      if (context.extractionActive)
      {
        AddReason(extract, "extraction_active", 62.0, "Extraction is active, so getting out matters now.");
      }
      if (context.timedMissionPressure)
      {
        AddReason(extract, "time_pressure", 15.0, "The timer pushes extraction to the front.");
      }
      if (helpAtExtraction)
      {
        AddReason(extract, "help_at_extraction", 12.0, "Help At Extraction boosts exit support.");
      }
      if (stance.extractionBias != 0.0)
      {
        AddReason(extract, "stance_extraction_bias", stance.extractionBias * 6.0, "This stance likes clean exits.");
      }
      if (!context.extractionActive)
      {
        AddReason(extract, "extraction_inactive", -20.0, "Extraction is not active yet.");
      }
      candidates.push_back(extract);

      auto evade = MakeCandidate("evade", "goal.regroup_safely");
      if (context.companionHealthPercent <= 40)
      {
        AddReason(evade, "low_companion_health", 48.0, "Low companion health makes retreat safer.");
      }
      if (context.unsafeToAdvance)
      {
        AddReason(evade, "unsafe_to_advance", 18.0, "The route ahead is too risky.");
      }
      if (retreatWhenHurt)
      {
        AddReason(evade, "retreat_when_hurt", 10.0, "Retreat When Hurt makes fallback choices easier.");
      }
      if (stance.cautionBias != 0.0)
      {
        AddReason(evade, "stance_caution_bias", stance.cautionBias * 6.0, "The active stance rewards safer regroups.");
      }
      if (context.playerNeedsRevive)
      {
        AddReason(evade, "revive_penalty", -18.0, "Backing away is less useful while the player needs rescue.");
      }
      candidates.push_back(evade);

      auto attack = MakeCandidate("attack", "goal.defend_player");
      if (context.threatVisible)
      {
        AddReason(attack, "threat_visible", 50.0, "A visible threat makes defense the clearest choice.");
      }
      if (context.sameTargetMarked)
      {
        AddReason(attack, "same_target_marked", 10.0, "The player marked the same target.");
      }
      if (protectMeFirst)
      {
        AddReason(attack, "protect_me_first", 7.0, "Protect Me First keeps pressure on nearby threats.");
      }
      if (context.playerLowHealth)
      {
        AddReason(attack, "player_low_health", 6.0, "Player health is low, so threats need to be checked.");
      }
      if (stance.attackBias != 0.0)
      {
        AddReason(attack, "stance_attack_bias", stance.attackBias * 6.0, "The active stance makes attack plans more attractive.");
      }
      if (context.companionHealthPercent <= 35)
      {
        AddReason(attack, "hurt_penalty", -10.0, "Low companion health makes direct attacks less safe.");
      }
      if (!context.threatVisible)
      {
        AddReason(attack, "no_visible_threat", -22.0, "There is no visible threat to attack.");
      }
      candidates.push_back(attack);

      auto loot = MakeCandidate("loot", "goal.recover_rare_loot");
      if (context.rareLootVisible)
      {
        AddReason(loot, "rare_loot_visible", 36.0, "A rare pickup is visible in this room.");
      }
      if (context.lootPingUnlocked)
      {
        AddReason(loot, "loot_ping", 8.0, "Loot Ping helps the companion handle item support clearly.");
      }
      if (grabRareLoot)
      {
        AddReason(loot, "grab_rare_loot", 10.0, "Grab Rare Loot makes item support more eager.");
      }
      if (stance.lootBias != 0.0)
      {
        AddReason(loot, "stance_loot_bias", stance.lootBias * 6.0, "The active stance rewards salvage opportunities.");
      }
      if (context.threatVisible)
      {
        AddReason(loot, "combat_penalty", -18.0, "Loot drops in priority while a threat is visible.");
      }
      if (context.extractionActive)
      {
        AddReason(loot, "extraction_penalty", -14.0, "The exit matters more than optional loot right now.");
      }
      if (context.distanceToPlayerMeters > static_cast<int>(followDistance) + 1)
      {
        AddReason(loot, "spacing_penalty", -6.0, "The companion is already far enough from the player.");
      }
      candidates.push_back(loot);

      auto moveTo = MakeCandidate("move_to", GoalForAction("move_to", context));
      if (context.distanceToPlayerMeters > static_cast<int>(followDistance))
      {
        AddReason(
          moveTo,
          "outside_follow_distance",
          28.0 + (context.distanceToPlayerMeters - followDistance),
          "The companion is outside the follow distance.");
      }
      if (context.currentGoal == "goal.scout_path")
      {
        AddReason(moveTo, "scout_goal", 14.0, "The current goal is to scout the next path.");
      }
      if (!context.lastKnownThreatPositionToken.empty() && !context.threatVisible)
      {
        AddReason(moveTo, "threat_memory", 8.0, "Threat memory still points to a useful position.");
      }
      if (config.holdPosition)
      {
        AddReason(moveTo, "hold_penalty", -40.0, "Hold position suppresses movement plans.");
      }
      candidates.push_back(moveTo);

      auto interact = MakeCandidate("interact", context.currentGoal.empty() ? "goal.interact" : context.currentGoal);
      const bool objectiveGoal = context.currentGoal.find("objective") != std::string::npos ||
        context.currentGoal.find("interact") != std::string::npos;
      if (objectiveGoal)
      {
        AddReason(interact, "objective_goal", 24.0, "The current goal wants an interaction.");
        if (!context.threatVisible)
        {
          AddReason(interact, "safe_window", 8.0, "There is a safe enough moment to interact.");
        }
      }
      if (context.extractionActive)
      {
        AddReason(interact, "extraction_penalty", -10.0, "Extraction is more urgent than interaction.");
      }
      candidates.push_back(interact);

      auto hold = MakeCandidate("hold", "goal.hold_position");
      if (config.holdPosition)
      {
        AddReason(hold, "hold_command", 88.0, "The player asked the companion to hold position.");
      }
      if (context.threatVisible)
      {
        AddReason(hold, "hold_with_threat", 4.0, "Holding can still cover the current lane.");
      }
      candidates.push_back(hold);

      auto wait = MakeCandidate("wait", "goal.wait");
      AddReason(wait, "default_wait", 5.0, "Nothing else is urgent, so waiting is safe.");
      candidates.push_back(wait);

      for (auto& candidate : candidates)
      {
        if (!blackboard.lastFailedActionId.empty() && candidate.actionId == blackboard.lastFailedActionId)
        {
          AddReason(candidate, "failed_recently", -18.0, "That action just failed, so the selector tries something else.");
        }
        if (perception.alertLevel == AlertLevel::Critical && candidate.actionId == "wait")
        {
          AddReason(candidate, "critical_alert_penalty", -12.0, "Waiting is risky while alert pressure is critical.");
        }
      }

      SortCandidates(candidates);
      return candidates;
    }

    std::vector<DecisionCandidate> BuildEnemyCandidates(
      const EnemyUnit& enemy,
      const CompanionWorldContext& context,
      const PerceptionSnapshot& perception,
      const AgentBlackboardState& blackboard)
    {
      const auto* archetype = FindEnemyArchetype(enemy.archetypeId);
      const auto chaseBias = archetype == nullptr ? 0.0 : archetype->chaseBias;
      const auto ambushBias = archetype == nullptr ? 0.0 : archetype->ambushBias;
      const auto patrolBias = archetype == nullptr ? 0.0 : archetype->patrolBias;
      const auto investigateBias = archetype == nullptr ? 0.0 : archetype->investigateBias;
      const auto returnBias = archetype == nullptr ? 0.0 : archetype->returnBias;
      const auto preferredRange = archetype == nullptr ? 4 : archetype->preferredRangeMeters;

      std::vector<DecisionCandidate> candidates;

      auto patrol = MakeCandidate("move_to", "goal.patrol_route");
      if (!context.threatVisible && context.heardEventToken.empty() && !enemy.alerted)
      {
        AddReason(patrol, "calm_patrol", 40.0, "No threat is visible, so the route resumes.");
      }
      AddReason(patrol, "patrol_bias", patrolBias * 6.0, "This archetype likes to keep moving on patrol.");
      candidates.push_back(patrol);

      auto investigate = MakeCandidate("move_to", "goal.investigate_signal");
      if (!context.heardEventToken.empty() || !context.lastKnownThreatPositionToken.empty())
      {
        AddReason(investigate, "heard_or_memory", 46.0, "A heard event or threat memory needs checking.");
      }
      AddReason(
        investigate,
        "investigate_bias",
        investigateBias * 6.0,
        "This archetype is willing to investigate suspicious signals.");
      candidates.push_back(investigate);

      auto alert = MakeCandidate("wait", "goal.raise_alert");
      if (context.threatVisible && !enemy.alerted)
      {
        AddReason(alert, "fresh_contact", 54.0, "A fresh target raises the alert state.");
      }
      if (enemy.variant == EnemyVariant::AlarmSupport)
      {
        AddReason(alert, "alarm_support_bias", 10.0, "Alarm support units like to escalate quickly.");
      }
      candidates.push_back(alert);

      auto chase = MakeCandidate("move_to", "goal.chase_target");
      if (context.threatVisible && context.distanceToThreatMeters > preferredRange)
      {
        AddReason(chase, "threat_out_of_range", 58.0, "The target is visible but still outside preferred range.");
      }
      AddReason(chase, "chase_bias", chaseBias * 6.0, "This archetype keeps pressure on moving targets.");
      candidates.push_back(chase);

      auto attack = MakeCandidate("attack", "goal.attack_target");
      if (context.threatVisible && context.distanceToThreatMeters <= preferredRange)
      {
        AddReason(attack, "target_in_range", 62.0, "The target is close enough to attack now.");
      }
      if (enemy.variant == EnemyVariant::AlarmSupport && context.playerInCover)
      {
        AddReason(attack, "cover_pressure", 10.0, "Ambush support units pressure covered targets.");
      }
      AddReason(attack, "ambush_bias", ambushBias * 6.0, "This archetype rewards ranged pressure and support.");
      candidates.push_back(attack);

      auto retreat = MakeCandidate("evade", "goal.regroup");
      if (enemy.health <= 8)
      {
        AddReason(retreat, "low_health", 48.0, "Low enemy health triggers a fallback.");
      }
      if (context.unsafeToAdvance && enemy.variant == EnemyVariant::AlarmSupport)
      {
        AddReason(retreat, "unsafe_lane", 10.0, "The lane ahead is unsafe for this unit.");
      }
      candidates.push_back(retreat);

      auto returnToRoute = MakeCandidate("move_to", "goal.return_to_route");
      if (!context.threatVisible && enemy.alerted)
      {
        AddReason(returnToRoute, "alert_decay", 34.0, "The threat was lost, so the unit returns to route.");
      }
      AddReason(returnToRoute, "return_bias", returnBias * 6.0, "This archetype knows how to reset cleanly.");
      candidates.push_back(returnToRoute);

      for (auto& candidate : candidates)
      {
        if (!blackboard.lastFailedActionId.empty() && candidate.actionId == blackboard.lastFailedActionId)
        {
          AddReason(candidate, "failed_recently", -16.0, "That action just failed, so the unit replans.");
        }
        if (perception.alertLevel == AlertLevel::Critical && candidate.actionId == "move_to" &&
          candidate.goalId == "goal.patrol_route")
        {
          AddReason(candidate, "critical_alert_penalty", -25.0, "Patrol is suppressed while alert pressure is high.");
        }
      }

      SortCandidates(candidates);
      return candidates;
    }

    std::string EnemyStateForCandidate(const DecisionCandidate& candidate)
    {
      if (candidate.goalId == "goal.patrol_route")
      {
        return "patrol";
      }
      if (candidate.goalId == "goal.investigate_signal")
      {
        return "investigate";
      }
      if (candidate.goalId == "goal.raise_alert")
      {
        return "alert";
      }
      if (candidate.goalId == "goal.chase_target")
      {
        return "chase";
      }
      if (candidate.goalId == "goal.attack_target")
      {
        return "attack";
      }
      if (candidate.goalId == "goal.regroup")
      {
        return "retreat";
      }
      return "return_to_route";
    }
  } // namespace

  bool HasLineOfSight(const int distanceToTargetMeters, const int maxSightMeters, const bool occluded)
  {
    return !occluded && distanceToTargetMeters <= maxSightMeters;
  }

  bool CanHearEvent(const int loudness, const int distanceToEventMeters, const int hearingThreshold)
  {
    return distanceToEventMeters <= loudness + hearingThreshold;
  }

  std::vector<ThreatMemoryEntry> DecayThreatMemory(std::vector<ThreatMemoryEntry> memory, const int turns)
  {
    for (auto& entry : memory)
    {
      entry.ageTurns += turns;
      entry.confidence = std::max(0.0, entry.confidence - (0.15 * turns));
      entry.urgencyLevel = std::max(0, entry.urgencyLevel - turns);
    }

    memory.erase(
      std::remove_if(
        memory.begin(),
        memory.end(),
        [](const ThreatMemoryEntry& entry) {
          return entry.confidence <= 0.0 || entry.ageTurns > 4;
        }),
      memory.end());
    return memory;
  }

  double ResolveFollowDistance(const CompanionConfig& config)
  {
    const auto* chip = FindBehaviorChip(kStayNearChipId);
    const auto minimum = chip == nullptr ? 4.0 : chip->minValue;
    const auto maximum = chip == nullptr ? 10.0 : chip->maxValue;
    return std::clamp(ChipValue(config, kStayNearChipId), minimum, maximum);
  }

  CompanionConfig DefaultCompanionConfig()
  {
    return CompanionConfig{};
  }

  PerceptionSnapshot BuildPerceptionSnapshot(const CompanionWorldContext& context, const std::string_view agentId)
  {
    (void)agentId;
    PerceptionSnapshot snapshot;
    snapshot.roomNodeId = context.roomNodeId;
    snapshot.routeNodeId = context.routeNodeId;

    if (context.threatVisible && HasLineOfSight(context.distanceToThreatMeters, 16, false))
    {
      snapshot.stimuli.push_back(PerceptionStimulus{
        "stimulus.sight.threat",
        StimulusKind::LineOfSight,
        context.visibleThreatId,
        context.lastKnownThreatPositionToken.empty() ? context.roomNodeId : context.lastKnownThreatPositionToken,
        std::max(2, context.urgencyLevel),
        0.95,
        "A threat is visible in line of sight."});
    }

    if (!context.heardEventToken.empty() &&
      CanHearEvent(std::max(2, context.urgencyLevel + 2), context.distanceToThreatMeters, 4))
    {
      snapshot.stimuli.push_back(PerceptionStimulus{
        "stimulus.sound.event",
        StimulusKind::HeardEvent,
        context.currentTargetId,
        context.routeNodeId,
        std::max(1, context.urgencyLevel),
        0.65,
        "A heard event suggests activity nearby."});
    }

    if (context.rareLootVisible || context.interestMarkerActive || !context.interestMarkerId.empty())
    {
      snapshot.stimuli.push_back(PerceptionStimulus{
        "stimulus.interest.marker",
        StimulusKind::InterestMarker,
        context.interestMarkerId.empty() ? "marker.rare_loot" : context.interestMarkerId,
        context.roomNodeId,
        1,
        0.55,
        "An interest marker is asking for attention."});
    }

    if (context.threatVisible || !context.lastKnownThreatPositionToken.empty())
    {
      snapshot.rememberedThreats.push_back(ThreatMemoryEntry{
        context.visibleThreatId.empty() ? context.currentTargetId : context.visibleThreatId,
        context.lastKnownThreatPositionToken.empty() ? context.roomNodeId : context.lastKnownThreatPositionToken,
        0,
        std::max(1, context.urgencyLevel),
        context.threatVisible ? 0.95 : 0.45});
    }

    if (context.playerNeedsRevive || (context.extractionActive && context.timedMissionPressure) ||
      (context.threatVisible && context.unsafeToAdvance))
    {
      snapshot.alertLevel = AlertLevel::Critical;
    }
    else if (context.threatVisible || !context.heardEventToken.empty())
    {
      snapshot.alertLevel = AlertLevel::Alert;
    }
    else if (context.rareLootVisible || context.interestMarkerActive)
    {
      snapshot.alertLevel = AlertLevel::Curious;
    }
    else
    {
      snapshot.alertLevel = AlertLevel::Calm;
    }

    snapshot.noticedReason = snapshot.stimuli.empty()
      ? "No active stimuli."
      : snapshot.stimuli.front().reasonText;
    return snapshot;
  }

  AgentExplainSnapshot BuildExplainSnapshot(const CompanionDecisionSnapshot& snapshot)
  {
    return AgentExplainSnapshot{
      "agent.companion.default",
      snapshot.currentGoal,
      snapshot.lastCompletedAction,
      snapshot.topReason,
      snapshot.secondaryReason,
      snapshot.confidenceLabel,
      snapshot.riskIndicator,
      snapshot.editDelta,
      snapshot.blackboard.routeNodeId,
      snapshot.blackboard.lastFailedActionId,
      snapshot.perception,
      snapshot.blackboard,
      snapshot.topCandidates,
      snapshot.stanceModifiers,
      snapshot.chipModifiers};
  }

  AgentExplainSnapshot BuildExplainSnapshot(const EnemyDecisionSnapshot& snapshot)
  {
    return AgentExplainSnapshot{
      "agent.enemy.default",
      snapshot.plan.goalId,
      snapshot.lastAction,
      snapshot.topReason,
      snapshot.secondaryReason,
      snapshot.confidenceLabel,
      std::string(ToString(snapshot.perception.alertLevel)),
      "Enemy archetypes do not expose child-facing edit deltas in Phase 3.",
      snapshot.blackboard.routeNodeId,
      snapshot.blackboard.lastFailedActionId,
      snapshot.perception,
      snapshot.blackboard,
      snapshot.topCandidates,
      {},
      {}};
  }

  CompanionDecisionSnapshot EvaluateCompanion(const CompanionConfig& rawConfig, const CompanionWorldContext& context)
  {
    return EvaluateCompanion(rawConfig, context, BehaviorOverrideSet{});
  }

  CompanionDecisionSnapshot EvaluateCompanion(
    const CompanionConfig& rawConfig,
    const CompanionWorldContext& context,
    const BehaviorOverrideSet& overrides)
  {
    const auto config = NormalizeConfig(rawConfig);
    const auto perception = BuildPerceptionSnapshot(context, "agent.companion.default");
    const auto blackboard = BuildBlackboard(config, context, perception);
    auto candidates = BuildCompanionCandidates(config, context, perception, blackboard);
    ApplyOverrides(candidates, overrides);
    SortCandidates(candidates);
    const auto& chosen = candidates.front();
    const auto [calloutToken, gestureToken] = FeedbackTokens(chosen.actionId);

    CompanionDecisionSnapshot snapshot;
    snapshot.currentState = CompanionStateForAction(chosen.actionId, context, config);
    snapshot.currentGoal = chosen.goalId;
    snapshot.lastAction = chosen.actionId;
    snapshot.lastCompletedAction = chosen.actionId;
    snapshot.topReason = BestReasonText(chosen);
    snapshot.secondaryReason = SecondaryReasonText(chosen);
    snapshot.influentialVariableA = "stance=" + config.stanceId;
    snapshot.influentialVariableB = "follow_distance=" + std::to_string(ResolveFollowDistance(config));
    snapshot.confidenceLabel = ConfidenceLabel(candidates);
    snapshot.riskIndicator = RiskIndicator(context);
    snapshot.editDelta = EditDelta(config);
    snapshot.calloutToken = calloutToken;
    snapshot.gestureToken = gestureToken;
    snapshot.perception = perception;
    snapshot.blackboard = blackboard;
    snapshot.topCandidates = candidates;
    snapshot.stanceModifiers = StanceModifierText(SafeStance(config.stanceId));
    snapshot.chipModifiers = ChipModifierText(config);
    snapshot.plan = ActionPlan{
      chosen.actionId,
      chosen.goalId,
      context.threatVisible ? context.visibleThreatId : context.currentTargetId,
      context.routeNodeId,
      {
        "stance:" + config.stanceId,
        "alert:" + std::string(ToString(perception.alertLevel)),
        "goal:" + chosen.goalId
      }};
    snapshot.lastResult = ActionResult{
      chosen.actionId,
      ActionExecutionStage::Succeeded,
      true,
      true,
      {},
      "Action executed in the deterministic shell."};

    std::ostringstream debug;
    debug << snapshot.currentState << "|" << snapshot.lastAction << "|" << JoinTopReasons(snapshot.topCandidates)
          << "|alert=" << ToString(snapshot.perception.alertLevel)
          << "|route=" << snapshot.blackboard.routeNodeId;
    snapshot.debugSummary = debug.str();
    return snapshot;
  }

  EnemyDecisionSnapshot EvaluateEnemy(const EnemyUnit& enemy, const CompanionWorldContext& context)
  {
    const auto perception = BuildPerceptionSnapshot(context, enemy.enemyId);

    AgentBlackboardState blackboard;
    blackboard.currentTargetId = context.threatVisible ? context.visibleThreatId : context.currentTargetId;
    blackboard.lastKnownThreatPositionToken = context.lastKnownThreatPositionToken;
    blackboard.currentGoalId = "goal.patrol_route";
    blackboard.healthConfidence = std::clamp(enemy.health / 20.0, 0.0, 1.0);
    blackboard.playerState = context.playerLowHealth ? "low_health" : "stable";
    blackboard.activeStanceId = enemy.archetypeId;
    blackboard.extractionUrgency = context.extractionUrgency;
    blackboard.lastFailedActionId = context.lastFailedAction;
    blackboard.routeNodeId = enemy.patrolRouteId;
    blackboard.roomNodeId = enemy.roomId;
    blackboard.alertLevel = std::string(ToString(perception.alertLevel));

    auto candidates = BuildEnemyCandidates(enemy, context, perception, blackboard);
    const auto& chosen = candidates.front();

    EnemyDecisionSnapshot snapshot;
    snapshot.currentState = EnemyStateForCandidate(chosen);
    snapshot.lastAction = chosen.actionId;
    snapshot.topReason = BestReasonText(chosen);
    snapshot.secondaryReason = SecondaryReasonText(chosen);
    snapshot.confidenceLabel = ConfidenceLabel(candidates);
    snapshot.perception = perception;
    snapshot.blackboard = blackboard;
    snapshot.topCandidates = candidates;
    snapshot.plan = ActionPlan{
      chosen.actionId,
      chosen.goalId,
      context.threatVisible ? context.visibleThreatId : context.currentTargetId,
      enemy.patrolRouteId,
      {
        "archetype:" + enemy.archetypeId,
        "alert:" + std::string(ToString(perception.alertLevel))
      }};
    snapshot.lastResult = ActionResult{
      chosen.actionId,
      ActionExecutionStage::Succeeded,
      true,
      true,
      {},
      "Enemy action resolved in the deterministic shell."};
    return snapshot;
  }

  std::string RenderExplainText(const CompanionDecisionSnapshot& snapshot)
  {
    std::ostringstream output;
    output << "Goal: " << snapshot.currentGoal << "\n"
           << "Last action: " << snapshot.lastCompletedAction << "\n"
           << "Why now:\n"
           << "- " << snapshot.topReason << "\n"
           << "- " << snapshot.secondaryReason << "\n"
           << "Confidence: " << snapshot.confidenceLabel << "\n"
           << "Risk: " << snapshot.riskIndicator << "\n"
           << "What changed after your edit: " << snapshot.editDelta;
    return output.str();
  }

  Peter::Core::StructuredFields ToSaveFields(const CompanionConfig& rawConfig)
  {
    const auto config = NormalizeConfig(rawConfig);
    return Peter::Core::StructuredFields{
      {"schema_version", "3"},
      {"stance_id", config.stanceId},
      {"active_chip_ids", SerializeStrings(config.activeChipIds)},
      {"chip_values", SerializeChipValues(config.chipValues)},
      {"follow_distance_meters", std::to_string(ResolveFollowDistance(config))}
    };
  }

  CompanionConfig CompanionConfigFromSaveFields(const Peter::Core::StructuredFields& fields)
  {
    CompanionConfig config = DefaultCompanionConfig();
    const auto schemaVersion = fields.find("schema_version");
    const auto distance = fields.find("follow_distance_meters");
    const auto hold = fields.find("hold_position");
    const auto stanceId = fields.find("stance_id");
    const auto activeChipIds = fields.find("active_chip_ids");
    const auto chipValues = fields.find("chip_values");

    if (schemaVersion == fields.end() || schemaVersion->second == "2")
    {
      if (distance != fields.end())
      {
        config.followDistanceMeters = std::stod(distance->second);
        config.chipValues[std::string(kStayNearChipId)] = config.followDistanceMeters;
      }
      config.stanceId = std::string(kDefaultStanceId);
      config.activeChipIds = {std::string(kStayNearChipId)};
      config.holdPosition = hold != fields.end() && hold->second == "true";
      return NormalizeConfig(config);
    }

    if (distance != fields.end())
    {
      config.followDistanceMeters = std::stod(distance->second);
      config.chipValues[std::string(kStayNearChipId)] = config.followDistanceMeters;
    }
    if (stanceId != fields.end())
    {
      config.stanceId = stanceId->second;
    }
    if (activeChipIds != fields.end())
    {
      config.activeChipIds = SplitStrings(activeChipIds->second);
    }
    if (chipValues != fields.end())
    {
      config.chipValues = ParseChipValues(chipValues->second);
    }
    config.holdPosition = false;
    return NormalizeConfig(config);
  }

  const std::vector<BehaviorStanceDefinition>& BuildPhase3Stances()
  {
    static const std::vector<BehaviorStanceDefinition> stances = {
      {"stance.cautious", "Cautious", "Stays safer and retreats earlier.", -0.5, 0.5, -0.5, 0.5, 0.5, 1.5},
      {"stance.balanced", "Balanced", "Keeps support, safety, and loot in a readable middle lane.", 0.0, 0.0, 0.0, 0.0, 0.0, 0.0},
      {"stance.aggressive", "Aggressive", "Pushes attack and pressure plans harder.", 1.8, 0.2, -0.3, -0.2, -0.2, -1.0},
      {"stance.scavenger", "Scavenger", "Prioritizes rare loot and optional rewards when it is safe.", -0.2, -0.1, 1.8, 0.0, 0.0, 0.2},
      {"stance.guardian", "Guardian", "Pulls the companion toward defense, rescue, and safe exits.", 0.4, 1.7, -0.2, 1.4, 0.8, 0.8}
    };
    return stances;
  }

  const BehaviorStanceDefinition* FindBehaviorStance(const std::string_view stanceId)
  {
    for (const auto& stance : BuildPhase3Stances())
    {
      if (stance.id == stanceId)
      {
        return &stance;
      }
    }
    return nullptr;
  }

  const std::vector<BehaviorChipDefinition>& BuildPhase3BehaviorChips()
  {
    static const std::vector<BehaviorChipDefinition> chips = {
      {
        "chip.stay_near_me",
        "Stay Near Me",
        "Tune how much space the companion gives the player.",
        "I will stay close enough to help quickly.",
        "Smaller spacing can make scouting slower.",
        "follow_distance_meters",
        6.0,
        4.0,
        10.0
      },
      {
        "chip.protect_me_first",
        "Protect Me First",
        "Raise defend and rescue choices when the player is under pressure.",
        "I will help the player before optional tasks.",
        "I may ignore nearby loot while danger is active.",
        "protect_bias",
        1.0,
        0.0,
        1.0
      },
      {
        "chip.grab_rare_loot",
        "Grab Rare Loot",
        "Raise rare-loot support and pickup decisions when the room is safe enough.",
        "I will notice and secure rare loot more often.",
        "I may step away for a short moment when the room is calm.",
        "loot_bias",
        1.0,
        0.0,
        1.0
      },
      {
        "chip.retreat_when_hurt",
        "Retreat When Hurt",
        "Raise fallback choices when the companion is damaged.",
        "I will pull back before I get knocked out.",
        "I may stop attacking sooner than usual.",
        "retreat_threshold",
        1.0,
        0.0,
        1.0
      },
      {
        "chip.help_at_extraction",
        "Help At Extraction",
        "Raise extraction support and end-of-mission cover behavior.",
        "I will focus on helping the team leave safely.",
        "I may skip late optional loot when extraction starts.",
        "extraction_bias",
        1.0,
        0.0,
        1.0
      }
    };
    return chips;
  }

  const BehaviorChipDefinition* FindBehaviorChip(const std::string_view chipId)
  {
    for (const auto& chip : BuildPhase3BehaviorChips())
    {
      if (chip.id == chipId)
      {
        return &chip;
      }
    }
    return nullptr;
  }

  const std::vector<EnemyArchetypeDefinition>& BuildPhase3EnemyArchetypes()
  {
    static const std::vector<EnemyArchetypeDefinition> archetypes = {
      {
        "enemy.archetype.machine_patrol",
        "Machine Patrol",
        "A route-driven machine that patrols, investigates, alerts, chases, and returns cleanly.",
        EnemyVariant::MeleeChaser,
        1.2,
        1.0,
        1.4,
        0.1,
        0.8,
        3
      },
      {
        "enemy.archetype.ranged_ambush_support",
        "Ranged Ambush Support",
        "A support machine that escalates fights, pressures cover, and favors ranged ambush windows.",
        EnemyVariant::AlarmSupport,
        0.3,
        0.8,
        0.9,
        1.7,
        0.5,
        7
      }
    };
    return archetypes;
  }

  const EnemyArchetypeDefinition* FindEnemyArchetype(const std::string_view archetypeId)
  {
    for (const auto& archetype : BuildPhase3EnemyArchetypes())
    {
      if (archetype.id == archetypeId)
      {
        return &archetype;
      }
    }
    return nullptr;
  }

  const std::vector<PatrolRouteDefinition>& BuildPhase3PatrolRoutes()
  {
    static const std::vector<PatrolRouteDefinition> routes = {
      {
        "route.machine_silo.entry_loop",
        "Entry Loop",
        {
          "room.raid.entry_platform",
          "room.raid.patrol_hall",
          "room.raid.guard_post"
        },
        true
      },
      {
        "route.machine_silo.vault_watch",
        "Vault Watch",
        {
          "room.raid.guard_post",
          "room.raid.high_risk_vault",
          "room.raid.coolant_walk"
        },
        true
      }
    };
    return routes;
  }

  const PatrolRouteDefinition* FindPatrolRoute(const std::string_view routeId)
  {
    for (const auto& route : BuildPhase3PatrolRoutes())
    {
      if (route.id == routeId)
      {
        return &route;
      }
    }
    return nullptr;
  }

  const std::vector<InterestMarkerDefinition>& BuildPhase3InterestMarkers()
  {
    static const std::vector<InterestMarkerDefinition> markers = {
      {"marker.rare_loot.vault_cache", "Vault Cache", "room.raid.high_risk_vault", "rare_loot", 1},
      {"marker.extraction.pad", "Extraction Pad", "room.raid.extraction_pad", "extraction", 3}
    };
    return markers;
  }

  const std::vector<AiScenarioDefinition>& BuildPhase3AiScenarios()
  {
    static const std::vector<AiScenarioDefinition> scenarios = {
      {
        "scenario.ai.follow_corridor.v1",
        "Follow Corridor",
        "Companion keeps up in a corridor and settles once spacing is safe again.",
        true,
        {
          {
            "step.follow_gap",
            CompanionWorldContext{
              false, false, false, false, false, false, false, false, false, false, false, false, false,
              9, 6, 100, 0, 0, "room.raid.patrol_hall", "route.machine_silo.entry_loop",
              "goal.follow_player", "player", "enemy.none", {}, {}, {}, {}
            },
            false,
            {},
            DefaultCompanionConfig(),
            "move_to",
            "follow"
          },
          {
            "step.follow_settle",
            CompanionWorldContext{
              false, false, false, false, false, false, false, false, false, false, false, false, false,
              4, 6, 100, 0, 0, "room.raid.patrol_hall", "route.machine_silo.entry_loop",
              "goal.follow_player", "player", "enemy.none", {}, {}, {}, {}
            },
            false,
            {},
            DefaultCompanionConfig(),
            "wait",
            "follow"
          }
        }
      },
      {
        "scenario.ai.enemy_ambush.v1",
        "Enemy Ambush",
        "The ambush support enemy investigates sound and then attacks from range.",
        true,
        {
          {
            "step.enemy_investigate",
            CompanionWorldContext{
              false, false, false, false, false, false, false, false, false, false, false, false, false,
              0, 7, 100, 0, 2, "room.raid.guard_post", "route.machine_silo.vault_watch",
              "goal.enemy_watch", "player", "enemy.none", "room.raid.guard_post", "sound.metal_ping", {}, {}
            },
            true,
            BuildEnemyUnit("enemy.machine_patrol.support_02", EnemyVariant::AlarmSupport, "room.raid.guard_post"),
            DefaultCompanionConfig(),
            "move_to",
            "investigate"
          },
          {
            "step.enemy_attack",
            CompanionWorldContext{
              true, false, false, false, false, false, false, false, false, false, false, true, false,
              0, 7, 100, 0, 3, "room.raid.guard_post", "route.machine_silo.vault_watch",
              "goal.enemy_pressure", "player", "player", "room.raid.guard_post", {}, {}, {}
            },
            true,
            BuildEnemyUnit("enemy.machine_patrol.support_02", EnemyVariant::AlarmSupport, "room.raid.guard_post"),
            DefaultCompanionConfig(),
            "attack",
            "attack"
          }
        }
      },
      {
        "scenario.ai.low_health_retreat.v1",
        "Low Health Retreat",
        "Companion retreats when health drops and the lane becomes unsafe.",
        true,
        {
          {
            "step.retreat",
            CompanionWorldContext{
              true, false, false, false, false, true, false, false, false, false, false, false, false,
              4, 5, 28, 0, 3, "room.raid.guard_post", "route.machine_silo.vault_watch",
              "goal.follow_player", "player", "enemy.machine_patrol.chaser_02", "room.raid.guard_post", {}, {}, {}
            },
            false,
            {},
            CompanionConfig{
              6.0,
              false,
              "stance.cautious",
              {"chip.stay_near_me", "chip.retreat_when_hurt"},
              {{"chip.stay_near_me", 6.0}, {"chip.retreat_when_hurt", 1.0}}
            },
            "evade",
            "retreat_regroup"
          }
        }
      },
      {
        "scenario.ai.revive_under_pressure.v1",
        "Revive Under Pressure",
        "Companion rescues the player instead of chasing more combat.",
        true,
        {
          {
            "step.revive",
            CompanionWorldContext{
              true, true, false, true, true, true, false, false, true, true, false, false, false,
              2, 4, 76, 0, 3, "room.raid.guard_post", "route.machine_silo.vault_watch",
              "goal.defend_player", "player", "enemy.machine_patrol.chaser_02", "room.raid.guard_post", {}, {}, {}
            },
            false,
            {},
            CompanionConfig{
              6.0,
              false,
              "stance.guardian",
              {"chip.stay_near_me", "chip.protect_me_first"},
              {{"chip.stay_near_me", 6.0}, {"chip.protect_me_first", 1.0}}
            },
            "revive",
            "revive_help"
          }
        }
      },
      {
        "scenario.ai.extraction_rush.v1",
        "Extraction Rush",
        "Companion prioritizes the exit when extraction pressure spikes.",
        true,
        {
          {
            "step.extract",
            CompanionWorldContext{
              false, false, true, true, false, false, false, true, false, false, false, false, true,
              3, 7, 82, 3, 3, "room.raid.coolant_walk", "route.machine_silo.vault_watch",
              "goal.reach_extraction", "player", "enemy.none", {}, "sound.extraction_alarm", "marker.extraction.pad", {}
            },
            false,
            {},
            CompanionConfig{
              6.0,
              false,
              "stance.guardian",
              {"chip.stay_near_me", "chip.help_at_extraction"},
              {{"chip.stay_near_me", 6.0}, {"chip.help_at_extraction", 1.0}}
            },
            "extract",
            "extract_assist"
          }
        }
      },
      {
        "scenario.ai.loot_vs_safety.v1",
        "Loot Versus Safety",
        "Companion attacks first, then pivots to loot once the room is safer.",
        true,
        {
          {
            "step.safety_first",
            CompanionWorldContext{
              true, true, false, false, false, false, true, false, false, false, true, false, true,
              5, 5, 88, 0, 2, "room.raid.high_risk_vault", "route.machine_silo.vault_watch",
              "goal.recover_rare_loot", "player", "enemy.machine_patrol.support_02",
              "room.raid.high_risk_vault", {}, "marker.rare_loot.vault_cache", {}
            },
            false,
            {},
            CompanionConfig{
              8.0,
              false,
              "stance.scavenger",
              {"chip.stay_near_me", "chip.grab_rare_loot"},
              {{"chip.stay_near_me", 8.0}, {"chip.grab_rare_loot", 1.0}}
            },
            "attack",
            "defend_player"
          },
          {
            "step.loot_after_clear",
            CompanionWorldContext{
              false, false, false, false, false, false, true, false, false, false, true, false, true,
              5, 5, 88, 0, 1, "room.raid.high_risk_vault", "route.machine_silo.vault_watch",
              "goal.recover_rare_loot", "player", "enemy.none", {}, {}, "marker.rare_loot.vault_cache", {}
            },
            false,
            {},
            CompanionConfig{
              8.0,
              false,
              "stance.scavenger",
              {"chip.stay_near_me", "chip.grab_rare_loot"},
              {{"chip.stay_near_me", 8.0}, {"chip.grab_rare_loot", 1.0}}
            },
            "loot",
            "loot_support"
          }
        }
      }
    };
    return scenarios;
  }

  const AiScenarioDefinition* FindAiScenario(const std::string_view scenarioId)
  {
    for (const auto& scenario : BuildPhase3AiScenarios())
    {
      if (scenario.id == scenarioId)
      {
        return &scenario;
      }
    }
    return nullptr;
  }

  std::string_view ToString(const StimulusKind kind)
  {
    switch (kind)
    {
      case StimulusKind::LineOfSight:
        return "line_of_sight";
      case StimulusKind::HeardEvent:
        return "heard_event";
      case StimulusKind::InterestMarker:
        return "interest_marker";
      case StimulusKind::ThreatMemory:
        return "threat_memory";
    }
    return "line_of_sight";
  }

  std::string_view ToString(const AlertLevel level)
  {
    switch (level)
    {
      case AlertLevel::Calm:
        return "calm";
      case AlertLevel::Curious:
        return "curious";
      case AlertLevel::Alert:
        return "alert";
      case AlertLevel::Critical:
        return "critical";
    }
    return "calm";
  }

  std::string_view ToString(const ActionExecutionStage stage)
  {
    switch (stage)
    {
      case ActionExecutionStage::Started:
        return "started";
      case ActionExecutionStage::Succeeded:
        return "succeeded";
      case ActionExecutionStage::Failed:
        return "failed";
      case ActionExecutionStage::Interrupted:
        return "interrupted";
    }
    return "started";
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
      variant == EnemyVariant::MeleeChaser
        ? "enemy.archetype.machine_patrol"
        : "enemy.archetype.ranged_ambush_support",
      variant == EnemyVariant::MeleeChaser
        ? "route.machine_silo.entry_loop"
        : "route.machine_silo.vault_watch",
      false,
      variant == EnemyVariant::MeleeChaser ? 24 : 18,
      true};
  }
} // namespace Peter::AI
