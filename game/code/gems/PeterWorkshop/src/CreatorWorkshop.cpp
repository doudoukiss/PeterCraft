#include "PeterWorkshop/CreatorWorkshop.h"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <cmath>
#include <sstream>
#include <stdexcept>

namespace Peter::Workshop
{
  namespace
  {
    std::string Trim(std::string value)
    {
      const auto notSpace = [](const unsigned char ch) {
        return !std::isspace(ch);
      };

      value.erase(
        value.begin(),
        std::find_if(value.begin(), value.end(), notSpace));
      value.erase(
        std::find_if(value.rbegin(), value.rend(), notSpace).base(),
        value.end());
      return value;
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

    template <typename T>
    std::string SerializeMap(const std::map<std::string, T, std::less<>>& values)
    {
      std::ostringstream output;
      bool first = true;
      for (const auto& [key, value] : values)
      {
        if (!first)
        {
          output << ",";
        }
        output << key << "=" << value;
        first = false;
      }
      return output.str();
    }

    std::string SerializeBoolMap(const std::map<std::string, bool, std::less<>>& values)
    {
      std::ostringstream output;
      bool first = true;
      for (const auto& [key, value] : values)
      {
        if (!first)
        {
          output << ",";
        }
        output << key << "=" << (value ? "true" : "false");
        first = false;
      }
      return output.str();
    }

    std::map<std::string, std::string, std::less<>> ParseStringMap(const std::string_view serialized)
    {
      std::map<std::string, std::string, std::less<>> values;
      for (const auto& token : SplitStrings(serialized))
      {
        const auto separator = token.find('=');
        if (separator == std::string::npos)
        {
          continue;
        }
        values[token.substr(0, separator)] = token.substr(separator + 1);
      }
      return values;
    }

    std::map<std::string, int, std::less<>> ParseIntMap(const std::string_view serialized)
    {
      std::map<std::string, int, std::less<>> values;
      for (const auto& token : SplitStrings(serialized))
      {
        const auto separator = token.find('=');
        if (separator == std::string::npos)
        {
          continue;
        }
        values[token.substr(0, separator)] = std::stoi(token.substr(separator + 1));
      }
      return values;
    }

    std::map<std::string, bool, std::less<>> ParseBoolMap(const std::string_view serialized)
    {
      std::map<std::string, bool, std::less<>> values;
      for (const auto& token : SplitStrings(serialized))
      {
        const auto separator = token.find('=');
        if (separator == std::string::npos)
        {
          continue;
        }
        values[token.substr(0, separator)] = token.substr(separator + 1) == "true";
      }
      return values;
    }

    bool EvaluateConditionId(const std::string_view conditionId, const Peter::AI::CompanionWorldContext& context)
    {
      if (conditionId == "player_health_low")
      {
        return context.playerLowHealth;
      }
      if (conditionId == "player_needs_revive")
      {
        return context.playerNeedsRevive;
      }
      if (conditionId == "rare_loot_visible")
      {
        return context.rareLootVisible;
      }
      if (conditionId == "extraction_active")
      {
        return context.extractionActive;
      }
      if (conditionId == "companion_health_low")
      {
        return context.companionHealthPercent <= 40;
      }
      if (conditionId == "timed_pressure_high")
      {
        return context.timedMissionPressure || context.extractionUrgency >= 3;
      }
      return false;
    }

    std::vector<Peter::AI::CandidateOverride> BuildOverridesForAction(const LogicCardDefinition& card)
    {
      const double boundedDelta = std::clamp(card.scoreDelta, -20.0, 20.0);

      if (card.actionId == "prioritize_help")
      {
        return {
          {card.id, "revive", boundedDelta, false, card.summary},
          {card.id, "attack", boundedDelta / 2.0, false, card.summary}
        };
      }
      if (card.actionId == "prioritize_cover")
      {
        return {
          {card.id, "hold", boundedDelta, false, card.summary},
          {card.id, "evade", boundedDelta / 2.0, false, card.summary}
        };
      }
      if (card.actionId == "mark_rare_loot")
      {
        return {{card.id, "loot", boundedDelta, false, card.summary}};
      }
      if (card.actionId == "retreat_regroup")
      {
        return {{card.id, "evade", boundedDelta, false, card.summary}};
      }
      if (card.actionId == "hold_position")
      {
        return {
          {card.id, "hold", boundedDelta, false, card.summary},
          {card.id, "move_to", 0.0, true, "Creator logic asked the companion to hold position."}
        };
      }
      if (card.actionId == "favor_extraction")
      {
        return {{card.id, "extract", boundedDelta, false, card.summary}};
      }
      return {};
    }

    std::map<std::string, double, std::less<>> ContextNumbers(const Peter::AI::CompanionWorldContext& context)
    {
      return {
        {"companion_health_percent", static_cast<double>(context.companionHealthPercent)},
        {"distance_to_player", static_cast<double>(context.distanceToPlayerMeters)},
        {"distance_to_threat", static_cast<double>(context.distanceToThreatMeters)},
        {"extraction_urgency", static_cast<double>(context.extractionUrgency)},
        {"follow_distance_meters", 6.0}
      };
    }

    std::map<std::string, bool, std::less<>> ContextBooleans(const Peter::AI::CompanionWorldContext& context)
    {
      return {
        {"companion_health_low", context.companionHealthPercent <= 40},
        {"extraction_active", context.extractionActive},
        {"player_health_low", context.playerLowHealth},
        {"player_needs_revive", context.playerNeedsRevive},
        {"rare_loot_visible", context.rareLootVisible},
        {"threat_visible", context.threatVisible},
        {"timed_pressure_high", context.timedMissionPressure || context.extractionUrgency >= 3},
        {"unsafe_to_advance", context.unsafeToAdvance}
      };
    }

    class TinyScriptRuntime
    {
    public:
      explicit TinyScriptRuntime(const Peter::AI::CompanionWorldContext& context)
        : m_numbers(ContextNumbers(context))
        , m_booleans(ContextBooleans(context))
      {
      }

      TinyScriptRunResult Run(const TinyScriptDefinition& script, const int stepBudget)
      {
        TinyScriptRunResult result;
        const auto lines = SplitLines(script.body);
        if (lines.empty())
        {
          result.error = "Script body cannot be empty.";
          return result;
        }
        if (static_cast<int>(lines.size()) > 20)
        {
          result.error = "Scripts are limited to 20 non-empty lines.";
          return result;
        }

        const auto startTime = std::chrono::steady_clock::now();
        for (std::size_t index = 0; index < lines.size(); ++index)
        {
          if (m_stepsUsed >= stepBudget)
          {
            result.error = "Script exceeded the 128-step evaluation budget.";
            return result;
          }
          if (std::chrono::steady_clock::now() - startTime > std::chrono::milliseconds(10))
          {
            result.error = "Script exceeded the 10ms runtime budget.";
            return result;
          }

          const auto& line = lines[index];
          if (line.starts_with("if "))
          {
            const auto returnPos = line.find(" return ");
            if (returnPos == std::string::npos)
            {
              result.error = "If statements must use the form: if <condition> return <value>.";
              return result;
            }

            if (EvaluateBoolean(Trim(line.substr(3, returnPos - 3))))
            {
              return Finish(Trim(line.substr(returnPos + 8)), result);
            }
            if (index + 1 < lines.size() && lines[index + 1].starts_with("else return "))
            {
              return Finish(Trim(lines[index + 1].substr(12)), result);
            }
            continue;
          }

          if (line.starts_with("else return "))
          {
            continue;
          }
          if (line.starts_with("return "))
          {
            return Finish(Trim(line.substr(7)), result);
          }

          result.error = "Only if/else and return statements are allowed in Phase 4 tiny scripts.";
          return result;
        }

        result.error = "Scripts must return a value.";
        return result;
      }

    private:
      std::vector<std::string> SplitLines(const std::string_view body) const
      {
        std::vector<std::string> lines;
        std::istringstream input{std::string(body)};
        std::string line;
        while (std::getline(input, line))
        {
          line = Trim(line);
          if (!line.empty())
          {
            lines.push_back(line);
          }
        }
        return lines;
      }

      bool EvaluateBoolean(const std::string& expression)
      {
        ++m_stepsUsed;

        const auto orPos = expression.find(" or ");
        if (orPos != std::string::npos)
        {
          return EvaluateBoolean(Trim(expression.substr(0, orPos))) ||
            EvaluateBoolean(Trim(expression.substr(orPos + 4)));
        }

        const auto andPos = expression.find(" and ");
        if (andPos != std::string::npos)
        {
          return EvaluateBoolean(Trim(expression.substr(0, andPos))) &&
            EvaluateBoolean(Trim(expression.substr(andPos + 5)));
        }

        if (expression.starts_with("not "))
        {
          return !EvaluateBoolean(Trim(expression.substr(4)));
        }

        static const std::vector<std::string> operators = {"<=", ">=", "==", "!=", "<", ">"};
        for (const auto& op : operators)
        {
          const auto opPos = expression.find(op);
          if (opPos == std::string::npos)
          {
            continue;
          }
          const auto left = EvaluateNumber(Trim(expression.substr(0, opPos)));
          const auto right = EvaluateNumber(Trim(expression.substr(opPos + op.size())));
          if (op == "<=")
          {
            return left <= right;
          }
          if (op == ">=")
          {
            return left >= right;
          }
          if (op == "==")
          {
            return std::fabs(left - right) < 0.0001;
          }
          if (op == "!=")
          {
            return std::fabs(left - right) >= 0.0001;
          }
          if (op == "<")
          {
            return left < right;
          }
          return left > right;
        }

        const auto iterator = m_booleans.find(expression);
        if (iterator != m_booleans.end())
        {
          return iterator->second;
        }
        throw std::runtime_error("Unknown boolean token: " + expression);
      }

      double EvaluateNumber(const std::string& expression)
      {
        ++m_stepsUsed;
        if (expression.starts_with("clamp(") && expression.ends_with(')'))
        {
          const auto inside = expression.substr(6, expression.size() - 7);
          const auto arguments = SplitArguments(inside);
          if (arguments.size() != 3)
          {
            throw std::runtime_error("clamp expects exactly three arguments.");
          }
          return std::clamp(
            EvaluateNumber(arguments[0]),
            EvaluateNumber(arguments[1]),
            EvaluateNumber(arguments[2]));
        }

        const auto numeric = m_numbers.find(expression);
        if (numeric != m_numbers.end())
        {
          return numeric->second;
        }
        if (expression == "true")
        {
          return 1.0;
        }
        if (expression == "false")
        {
          return 0.0;
        }
        return std::stod(expression);
      }

      std::vector<std::string> SplitArguments(const std::string_view text) const
      {
        std::vector<std::string> arguments;
        std::string current;
        int depth = 0;
        for (const char character : text)
        {
          if (character == '(')
          {
            ++depth;
          }
          else if (character == ')')
          {
            --depth;
          }
          else if (character == ',' && depth == 0)
          {
            arguments.push_back(Trim(current));
            current.clear();
            continue;
          }
          current += character;
        }
        if (!current.empty())
        {
          arguments.push_back(Trim(current));
        }
        return arguments;
      }

      TinyScriptRunResult Finish(const std::string& expression, TinyScriptRunResult& result)
      {
        try
        {
          if (expression.size() >= 2 && expression.front() == '"' && expression.back() == '"')
          {
            result.valid = true;
            result.returnedString = true;
            result.textResult = expression.substr(1, expression.size() - 2);
            result.stepsUsed = m_stepsUsed;
            return result;
          }

          result.valid = true;
          result.numericResult = EvaluateNumber(expression);
          result.stepsUsed = m_stepsUsed;
          return result;
        }
        catch (const std::exception& error)
        {
          result.valid = false;
          result.error = error.what();
          result.stepsUsed = m_stepsUsed;
          return result;
        }
      }

      std::map<std::string, double, std::less<>> m_numbers;
      std::map<std::string, bool, std::less<>> m_booleans;
      int m_stepsUsed = 0;
    };

  } // namespace

  const std::vector<TinkerVariableDefinition>& BuildPhase4TinkerVariables()
  {
    static const std::vector<TinkerVariableDefinition> variables = {
      {"companion.follow_distance_meters", "Follow Distance", "Tune how close the companion stays in all modes.", "slider", "group.companion", 6.0, 4.0, 10.0, false},
      {"companion.rare_loot_bias", "Rare Loot Focus", "Raise how strongly the companion notices rare loot in creator-safe content.", "slider", "group.companion", 0.0, 0.0, 1.0, true},
      {"creator.lesson_timer_seconds", "Lesson Timer", "Stretch lesson pacing in creator-safe content.", "slider", "group.creator", 60.0, 30.0, 120.0, true},
      {"creator.extraction_countdown_seconds", "Extraction Countdown", "Tune the creator-only extraction countdown.", "slider", "group.creator", 6.0, 4.0, 12.0, true},
      {"creator.learning_enemy_alert_delay_seconds", "Enemy Alert Delay", "Delay learning enemies a little in creator-safe content.", "slider", "group.creator", 3.0, 0.0, 8.0, true}
    };
    return variables;
  }

  const TinkerVariableDefinition* FindTinkerVariable(const std::string_view variableId)
  {
    for (const auto& variable : BuildPhase4TinkerVariables())
    {
      if (variable.id == variableId)
      {
        return &variable;
      }
    }
    return nullptr;
  }

  const std::vector<TinkerPresetDefinition>& BuildPhase4TinkerPresets()
  {
    static const std::vector<TinkerPresetDefinition> presets = {
      {"preset.tinker.companion_close_guard", "Close Guard", "Keep the companion close and protective.", "group.companion", {{"companion.follow_distance_meters", 4.0}, {"companion.rare_loot_bias", 0.1}}},
      {"preset.tinker.companion_balanced", "Balanced", "Keep the learning-friendly defaults.", "group.companion", {{"companion.follow_distance_meters", 6.0}, {"companion.rare_loot_bias", 0.35}}},
      {"preset.tinker.creator_roomy_teach", "Roomy Teach", "Slow the creator simulation down and leave more room to learn.", "group.creator", {{"creator.lesson_timer_seconds", 90.0}, {"creator.extraction_countdown_seconds", 9.0}, {"creator.learning_enemy_alert_delay_seconds", 5.0}}}
    };
    return presets;
  }

  const TinkerPresetDefinition* FindTinkerPreset(const std::string_view presetId)
  {
    for (const auto& preset : BuildPhase4TinkerPresets())
    {
      if (preset.id == presetId)
      {
        return &preset;
      }
    }
    return nullptr;
  }

  std::map<std::string, double, std::less<>> DefaultTinkerValues()
  {
    std::map<std::string, double, std::less<>> values;
    for (const auto& variable : BuildPhase4TinkerVariables())
    {
      values[variable.id] = variable.defaultValue;
    }
    return values;
  }

  Peter::AI::CompanionConfig ApplyTinkerValues(
    const Peter::AI::CompanionConfig& baseConfig,
    const std::map<std::string, double, std::less<>>& tinkerValues,
    const bool creatorScoped)
  {
    Peter::AI::CompanionConfig config = baseConfig;
    if (const auto followDistance = tinkerValues.find("companion.follow_distance_meters");
      followDistance != tinkerValues.end())
    {
      config.chipValues["chip.stay_near_me"] = followDistance->second;
    }

    if (creatorScoped)
    {
      if (const auto rareLootBias = tinkerValues.find("companion.rare_loot_bias");
        rareLootBias != tinkerValues.end() && rareLootBias->second > 0.6 &&
        std::find(config.activeChipIds.begin(), config.activeChipIds.end(), "chip.grab_rare_loot") ==
          config.activeChipIds.end())
      {
        config.activeChipIds.push_back("chip.grab_rare_loot");
      }
    }

    config.followDistanceMeters = Peter::AI::ResolveFollowDistance(config);
    return config;
  }

  Peter::AI::BehaviorOverrideSet BuildTinkerBehaviorOverrides(
    const std::map<std::string, double, std::less<>>& tinkerValues,
    const bool creatorScoped)
  {
    Peter::AI::BehaviorOverrideSet overrides;
    overrides.summary = "Tinker overrides";
    if (!creatorScoped)
    {
      return overrides;
    }

    if (const auto rareLootBias = tinkerValues.find("companion.rare_loot_bias");
      rareLootBias != tinkerValues.end())
    {
      overrides.overrides.push_back({"tinker.rare_loot_bias", "loot", rareLootBias->second * 12.0, false, "Tinker Mode raised the companion's rare-loot interest."});
    }

    if (const auto alertDelay = tinkerValues.find("creator.learning_enemy_alert_delay_seconds");
      alertDelay != tinkerValues.end() && alertDelay->second > 0.0)
    {
      overrides.overrides.push_back({"tinker.enemy_alert_delay", "attack", -std::min(10.0, alertDelay->second * 1.5), false, "Learning enemy alert delay softens early attack choices."});
      overrides.overrides.push_back({"tinker.enemy_alert_delay", "move_to", std::min(6.0, alertDelay->second), false, "Learning enemy alert delay gives scouting a little more room."});
    }

    return overrides;
  }

  const std::vector<LogicRulesetDefinition>& BuildPhase4LogicTemplates()
  {
    static const std::vector<LogicRulesetDefinition> templates = {
      {"logic.template.protect_player", "Protect Player", "Mirror the old Protect Me First chip with ordered cards.", {{"logic.card.protect_player.revive", "Revive First", "Rescue the player before anything else.", "player_needs_revive", "prioritize_help", 18.0, false}, {"logic.card.protect_player.low_health", "Cover When Hurt", "Favor cover when the player is already low.", "player_health_low", "prioritize_cover", 12.0, false}}},
      {"logic.template.rare_loot_helper", "Rare Loot Helper", "Mirror the old Grab Rare Loot chip with ordered cards.", {{"logic.card.rare_loot.mark", "Mark Rare Loot", "Spot rare loot when the room is safe enough.", "rare_loot_visible", "mark_rare_loot", 16.0, false}, {"logic.card.rare_loot.extract", "Do Not Greed During Exit", "Switch back to extraction once the exit is active.", "extraction_active", "favor_extraction", 10.0, false}}},
      {"logic.template.exit_helper", "Exit Helper", "Mirror the old Help At Extraction chip with ordered cards.", {{"logic.card.exit.extract", "Favor Extraction", "Help the player leave cleanly when the exit is active.", "extraction_active", "favor_extraction", 18.0, false}, {"logic.card.exit.retreat", "Regroup Under Timer", "Back off when the timer pressure gets high.", "timed_pressure_high", "retreat_regroup", 12.0, false}}}
    };
    return templates;
  }

  const LogicRulesetDefinition* FindLogicTemplate(const std::string_view rulesetId)
  {
    for (const auto& ruleset : BuildPhase4LogicTemplates())
    {
      if (ruleset.id == rulesetId)
      {
        return &ruleset;
      }
    }
    return nullptr;
  }

  Peter::AI::BehaviorOverrideSet CompileLogicRuleset(
    const LogicRulesetDefinition& ruleset,
    const Peter::AI::CompanionWorldContext& context)
  {
    Peter::AI::BehaviorOverrideSet overrides;
    overrides.summary = ruleset.displayName;

    const std::size_t limit = std::min<std::size_t>(5, ruleset.cards.size());
    for (std::size_t index = 0; index < limit; ++index)
    {
      const auto& card = ruleset.cards[index];
      if (!EvaluateConditionId(card.conditionId, context))
      {
        continue;
      }
      const auto cardOverrides = BuildOverridesForAction(card);
      overrides.overrides.insert(overrides.overrides.end(), cardOverrides.begin(), cardOverrides.end());
    }

    return overrides;
  }

  const std::vector<TinyScriptDefinition>& BuildPhase4TinyScriptTemplates()
  {
    static const std::vector<TinyScriptDefinition> scripts = {
      {"script.template.priority_hint", "Priority Hint", "Raise revive priority when the player needs help.", TinyScriptHookKind::CompanionPriorityHint, "if player_needs_revive return clamp(16, -20, 20)\nreturn 0", "revive"},
      {"script.template.message_override", "Lesson Message", "Swap in a more direct lesson message while the player is hurt.", TinyScriptHookKind::TutorialMessageOverride, "if player_health_low return \"Stay close and help first.\"\nreturn \"Keep exploring.\"", ""}
    };
    return scripts;
  }

  const TinyScriptDefinition* FindTinyScriptTemplate(const std::string_view scriptId)
  {
    for (const auto& script : BuildPhase4TinyScriptTemplates())
    {
      if (script.id == scriptId)
      {
        return &script;
      }
    }
    return nullptr;
  }

  TinyScriptRunResult ValidateTinyScript(const TinyScriptDefinition& script)
  {
    Peter::AI::CompanionWorldContext context;
    context.playerLowHealth = true;
    context.playerNeedsRevive = true;
    context.rareLootVisible = true;
    context.extractionActive = true;
    context.timedMissionPressure = true;
    context.companionHealthPercent = 35;
    context.distanceToPlayerMeters = 3;
    context.distanceToThreatMeters = 5;
    return RunTinyScript(script, context, 128);
  }

  TinyScriptRunResult RunTinyScript(
    const TinyScriptDefinition& script,
    const Peter::AI::CompanionWorldContext& context,
    const int stepBudget)
  {
    TinyScriptRuntime runtime(context);
    return runtime.Run(script, stepBudget);
  }

  Peter::AI::BehaviorOverrideSet BuildScriptBehaviorOverrides(
    const TinyScriptDefinition& script,
    const Peter::AI::CompanionWorldContext& context)
  {
    Peter::AI::BehaviorOverrideSet overrides;
    overrides.summary = script.displayName;

    if (script.hookKind != TinyScriptHookKind::CompanionPriorityHint)
    {
      return overrides;
    }

    const auto result = RunTinyScript(script, context, 128);
    if (!result.valid)
    {
      return overrides;
    }

    overrides.overrides.push_back({
      script.id,
      script.targetActionId.empty() ? "move_to" : script.targetActionId,
      std::clamp(result.numericResult, -20.0, 20.0),
      false,
      "Tiny Script nudged this action."});
    return overrides;
  }

  CreatorReplaySnippet BuildCreatorReplaySnippet(
    const std::string_view scenarioId,
    const std::vector<std::string>& beforeTimeline,
    const std::vector<std::string>& afterTimeline,
    const std::string_view changeSummary)
  {
    CreatorReplaySnippet snippet;
    snippet.scenarioId = std::string(scenarioId);
    snippet.beforeSummary = beforeTimeline.empty() ? "no baseline actions" : beforeTimeline.front();
    snippet.afterSummary = afterTimeline.empty() ? "no edited actions" : afterTimeline.front();
    snippet.changeSummary = std::string(changeSummary);

    const std::size_t count = std::max(beforeTimeline.size(), afterTimeline.size());
    for (std::size_t index = 0; index < count; ++index)
    {
      const std::string before = index < beforeTimeline.size() ? beforeTimeline[index] : "none";
      const std::string after = index < afterTimeline.size() ? afterTimeline[index] : "none";
      snippet.timeline.push_back("step " + std::to_string(index + 1) + ": before=" + before + " | after=" + after);
    }
    return snippet;
  }

  CreatorActivationResult ActivateCreatorArtifact(
    CreatorManifest& manifest,
    const std::string_view kindId,
    const std::string_view contentId,
    const int revision,
    const bool validationPassed)
  {
    const std::string key = std::string(kindId) + ":" + std::string(contentId);
    CreatorActivationResult result;

    if (!validationPassed)
    {
      result.fallbackRevision = manifest.lastKnownGoodRevisions[key];
      result.summary = result.fallbackRevision > 0
        ? "Activation failed validation. Reverted to the last known good revision."
        : "Activation failed validation and no last known good revision exists yet.";
      return result;
    }

    if (const auto previousId = manifest.activeDraftIds.find(std::string(kindId));
      previousId != manifest.activeDraftIds.end())
    {
      const std::string previousKey = previousId->first + ":" + previousId->second;
      if (const auto previousRevision = manifest.lastKnownGoodRevisions.find(previousKey);
        previousRevision != manifest.lastKnownGoodRevisions.end())
      {
        manifest.rollbackTargets[key] = previousRevision->second;
      }
    }

    manifest.activeDraftIds[std::string(kindId)] = std::string(contentId);
    manifest.lastKnownGoodRevisions[key] = revision;
    manifest.disabledContent[key] = false;

    result.success = true;
    result.activeRevision = revision;
    result.summary = "Creator content activated successfully.";
    return result;
  }

  void DisableCreatorArtifact(
    CreatorManifest& manifest,
    const std::string_view kindId,
    const std::string_view contentId)
  {
    manifest.disabledContent[std::string(kindId) + ":" + std::string(contentId)] = true;
  }

  Peter::Core::StructuredFields ToSaveFields(const CreatorManifest& manifest)
  {
    return Peter::Core::StructuredFields{
      {"schema_version", "4"},
      {"active_draft_ids", SerializeMap(manifest.activeDraftIds)},
      {"last_known_good_revisions", SerializeMap(manifest.lastKnownGoodRevisions)},
      {"disabled_content", SerializeBoolMap(manifest.disabledContent)},
      {"rollback_targets", SerializeMap(manifest.rollbackTargets)}
    };
  }

  CreatorManifest CreatorManifestFromSaveFields(const Peter::Core::StructuredFields& fields)
  {
    CreatorManifest manifest;
    if (const auto active = fields.find("active_draft_ids"); active != fields.end())
    {
      manifest.activeDraftIds = ParseStringMap(active->second);
    }
    if (const auto revisions = fields.find("last_known_good_revisions"); revisions != fields.end())
    {
      manifest.lastKnownGoodRevisions = ParseIntMap(revisions->second);
    }
    if (const auto disabled = fields.find("disabled_content"); disabled != fields.end())
    {
      manifest.disabledContent = ParseBoolMap(disabled->second);
    }
    if (const auto rollback = fields.find("rollback_targets"); rollback != fields.end())
    {
      manifest.rollbackTargets = ParseIntMap(rollback->second);
    }
    return manifest;
  }

  Peter::Core::StructuredFields ToSaveFields(const CreatorProgressState& progressState)
  {
    return Peter::Core::StructuredFields{
      {"schema_version", "4"},
      {"completed_creator_lessons", SerializeStrings(progressState.completedCreatorLessons)},
      {"mentor_view_unlocked", progressState.mentorViewUnlocked ? "true" : "false"},
      {"safe_simulation_runs", std::to_string(progressState.safeSimulationRuns)}
    };
  }

  CreatorProgressState CreatorProgressStateFromSaveFields(const Peter::Core::StructuredFields& fields)
  {
    CreatorProgressState progress;
    if (const auto lessons = fields.find("completed_creator_lessons"); lessons != fields.end())
    {
      progress.completedCreatorLessons = SplitStrings(lessons->second);
    }
    if (const auto mentor = fields.find("mentor_view_unlocked"); mentor != fields.end())
    {
      progress.mentorViewUnlocked = mentor->second == "true";
    }
    if (const auto runs = fields.find("safe_simulation_runs"); runs != fields.end())
    {
      progress.safeSimulationRuns = std::stoi(runs->second);
    }
    return progress;
  }

  Peter::Core::StructuredFields ToSaveFields(const CreatorSettings& settings)
  {
    return Peter::Core::StructuredFields{
      {"schema_version", "4"},
      {"creator_content_enabled", settings.creatorContentEnabled ? "true" : "false"},
      {"mentor_view_visible", settings.mentorViewVisible ? "true" : "false"},
      {"safe_simulation_enabled", settings.safeSimulationEnabled ? "true" : "false"}
    };
  }

  CreatorSettings CreatorSettingsFromSaveFields(const Peter::Core::StructuredFields& fields)
  {
    CreatorSettings settings;
    if (const auto enabled = fields.find("creator_content_enabled"); enabled != fields.end())
    {
      settings.creatorContentEnabled = enabled->second == "true";
    }
    if (const auto mentor = fields.find("mentor_view_visible"); mentor != fields.end())
    {
      settings.mentorViewVisible = mentor->second == "true";
    }
    if (const auto safe = fields.find("safe_simulation_enabled"); safe != fields.end())
    {
      settings.safeSimulationEnabled = safe->second == "true";
    }
    return settings;
  }

  std::string_view ToString(const TinyScriptHookKind hookKind)
  {
    switch (hookKind)
    {
      case TinyScriptHookKind::MissionScoreBonus:
        return "mission.score_bonus";
      case TinyScriptHookKind::CompanionPriorityHint:
        return "companion.priority_hint";
      case TinyScriptHookKind::TutorialMessageOverride:
        return "tutorial.message_override";
      case TinyScriptHookKind::LootRarityReaction:
        return "loot.rarity_reaction";
    }
    return "companion.priority_hint";
  }
} // namespace Peter::Workshop
