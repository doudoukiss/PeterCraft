#include "PeterUI/SlicePresentation.h"

#include <sstream>

namespace Peter::UI
{
  namespace
  {
    std::string LookupChildFacingText(const std::string_view textId)
    {
      static const std::map<std::string, std::string, std::less<>> kStrings = {
        {"help.first_mission", "Pick a mission with a clear goal and head out when you feel ready."},
        {"help.first_extraction", "Follow the bright extraction cue and stay calm during the countdown."},
        {"help.first_repair", "Broken gear can be repaired or recovered in the workshop."},
        {"help.first_creator_change", "Try a safe workshop change, preview it, and then watch what happens."},
        {"help.first_explain_panel", "Open the Why panel to see the top reasons behind your companion's choice."},
        {"input.crouch", "Crouch"},
        {"input.interact", "Interact"},
        {"input.jump", "Jump"},
        {"input.move", "Move"},
        {"input.sprint", "Sprint"},
        {"onboarding.first_creator_change", "First safe creator change"},
        {"onboarding.first_explain_panel", "First explain panel open"},
        {"onboarding.first_extraction", "First extraction"},
        {"onboarding.first_mission", "First mission launch"},
        {"onboarding.first_repair", "First repair or recovery"},
        {"workshop.validation.invalid", "That change is not safe yet. Try the suggested range or reset it."},
        {"workshop.validation.valid", "That change is safe. Preview it, then apply it when you are ready."}
      };

      const auto iterator = kStrings.find(std::string(textId));
      return iterator == kStrings.end() ? std::string(textId) : iterator->second;
    }

    std::string SerializeBindings(const std::map<std::string, std::string, std::less<>>& bindings)
    {
      std::ostringstream output;
      bool first = true;
      for (const auto& [actionId, binding] : bindings)
      {
        if (!first)
        {
          output << ",";
        }
        output << actionId << "=" << binding;
        first = false;
      }
      return output.str();
    }

    std::map<std::string, std::string, std::less<>> ParseBindings(std::string_view serialized)
    {
      std::map<std::string, std::string, std::less<>> bindings;
      std::size_t cursor = 0;
      while (cursor < serialized.size())
      {
        const auto separator = serialized.find(',', cursor);
        const auto token = serialized.substr(
          cursor,
          separator == std::string_view::npos ? serialized.size() - cursor : separator - cursor);
        if (!token.empty())
        {
          const auto equals = token.find('=');
          if (equals != std::string_view::npos)
          {
            bindings[std::string(token.substr(0, equals))] = std::string(token.substr(equals + 1));
          }
        }

        if (separator == std::string_view::npos)
        {
          break;
        }
        cursor = separator + 1;
      }
      return bindings;
    }
  } // namespace

  std::string RenderHomeBaseOverview(const Peter::World::HomeBaseDefinition& homeBase)
  {
    std::ostringstream output;
    output << homeBase.displayName << ": ";
    bool first = true;
    for (const auto& station : homeBase.stations)
    {
      if (!first)
      {
        output << " | ";
      }
      output << station.displayName;
      first = false;
    }
    return output.str();
  }

  std::string RenderMissionBoard(
    const std::vector<Peter::World::MissionTemplateDefinition>& missions,
    const std::string_view selectedMissionId)
  {
    std::ostringstream output;
    output << "Mission templates";
    for (const auto& mission : missions)
    {
      output << "\n- " << mission.displayName << " [" << mission.templateType << "]";
      if (mission.id == selectedMissionId)
      {
        output << " (selected)";
      }
      output << " reward_tip=" << mission.rewardBundle.lessonTipId;
    }
    return output.str();
  }

  std::string RenderMissionBlueprintPreview(
    const Peter::World::MissionBlueprintDefinition& blueprint,
    const Peter::World::RaidZoneDefinition& raidZone)
  {
    std::ostringstream output;
    output << "Mission graph\n"
           << blueprint.displayName << " [" << blueprint.templateFamilyId << "]\n"
           << "Zone: " << blueprint.zoneDisplayName << " scene=" << blueprint.sceneId
           << "\nRooms:";
    for (const auto& room : raidZone.rooms)
    {
      output << "\n- " << room.id << " -> exits=" << room.exits.size();
    }
    output << "\nFeedback tags:";
    for (const auto& tagId : blueprint.feedbackTagIds)
    {
      output << "\n- " << tagId;
    }
    return output.str();
  }

  std::string RenderEncounterPatternPreview(const Peter::World::EncounterPatternDefinition& encounter)
  {
    std::ostringstream output;
    output << "Encounter preview\n"
           << encounter.displayName << " room=" << encounter.roomId
           << "\nEnemies:";
    for (const auto& enemy : encounter.enemies)
    {
      output << "\n- " << enemy.enemyId;
    }
    output << "\nLoot anchors:";
    for (const auto& lootId : encounter.lootItemIds)
    {
      output << "\n- " << lootId;
    }
    output << "\nFeedback tags:";
    for (const auto& tagId : encounter.feedbackTagIds)
    {
      output << "\n- " << tagId;
    }
    return output.str();
  }

  std::string RenderRaidZoneOverview(
    const Peter::World::RaidZoneDefinition& raidZone,
    const Peter::World::MissionTemplateDefinition& mission)
  {
    std::ostringstream output;
    output << raidZone.displayName << " template=" << mission.templateType
           << " rooms=" << mission.roomIds.size()
           << " extraction=" << mission.extractionCountdownSeconds << "s";
    return output.str();
  }

  std::string RenderCompanionBehaviorEditor(const Peter::AI::CompanionConfig& config)
  {
    std::ostringstream output;
    output << "Companion terminal\n"
           << "Stance: " << config.stanceId << "\n"
           << "Active chips:";

    for (const auto& chipId : config.activeChipIds)
    {
      output << "\n- " << chipId;
    }

    output << "\nFollow distance: " << Peter::AI::ResolveFollowDistance(config) << "m";
    return output.str();
  }

  std::string RenderCompanionExplainPanel(const Peter::AI::CompanionDecisionSnapshot& snapshot)
  {
    return Peter::AI::RenderExplainText(snapshot);
  }

  std::string RenderCompanionCompareView(
    const Peter::AI::CompanionDecisionSnapshot& before,
    const Peter::AI::CompanionDecisionSnapshot& after,
    const std::string_view previewSummary)
  {
    std::ostringstream output;
    output << "What changed after your edit?\n"
           << "Before: " << before.currentState << " -> " << before.lastAction << "\n"
           << "After: " << after.currentState << " -> " << after.lastAction << "\n"
           << "Reason shift: " << after.topReason << "\n"
           << "Preview: " << previewSummary;
    return output.str();
  }

  std::string RenderAiDebugPanel(const Peter::AI::AgentExplainSnapshot& snapshot)
  {
    std::ostringstream output;
    output << "AI Debug\n"
           << "Goal: " << snapshot.currentGoal << "\n"
           << "Last action: " << snapshot.lastCompletedAction << "\n"
           << "Alert: " << snapshot.perception.noticedReason << " (" << snapshot.blackboard.alertLevel << ")\n"
           << "Route node: " << snapshot.routeNodeId << "\n"
           << "Last failure: "
           << (snapshot.lastActionFailureReason.empty() ? "none" : snapshot.lastActionFailureReason)
           << "\nTop actions:";

    for (const auto& candidate : snapshot.topCandidates)
    {
      output << "\n- " << candidate.actionId << " score=" << candidate.score;
    }

    output << "\nStance modifiers:";
    for (const auto& entry : snapshot.stanceModifiers)
    {
      output << "\n- " << entry;
    }

    output << "\nChip modifiers:";
    for (const auto& entry : snapshot.chipModifiers)
    {
      output << "\n- " << entry;
    }
    return output.str();
  }

  std::string RenderPostRaidSummary(const Peter::World::RaidSummary& summary)
  {
    std::ostringstream output;
    output << (summary.success ? "Raid success" : "Raid failed") << "\n"
           << "Mission: " << summary.missionDisplayName << "\n"
           << "Timeline:";
    for (const auto& entry : summary.timeline)
    {
      output << "\n- " << entry;
    }
    output << "\nGained: " << summary.gainedItems
           << "\nLost: " << summary.lostItems
           << "\nBroken: " << summary.brokenItems
           << "\nRecovered: " << summary.recoveredItems
           << "\nCompanion: " << summary.companionHighlight
           << "\nLesson: " << summary.lessonTip;
    return output.str();
  }

  std::string RenderTutorialLesson(
    const Peter::World::TutorialLessonDefinition& lesson,
    const int hintLevel)
  {
    std::ostringstream output;
    output << lesson.displayName << " | hint level " << hintLevel;
    for (const auto& step : lesson.steps)
    {
      output << "\n- " << step.action << ": " << step.prompt;
    }
    return output.str();
  }

  std::string ResolveChildFacingText(const std::string_view textId)
  {
    return LookupChildFacingText(textId);
  }

  std::string ResolveActionDisplayLabel(const Peter::Adapters::ActionBinding& binding)
  {
    return LookupChildFacingText(binding.displayLabelId.empty() ? binding.actionId : binding.displayLabelId);
  }

  std::string RenderAccessibilitySettings(
    const AccessibilitySettings& settings,
    const std::vector<Peter::Adapters::ActionBinding>& bindings)
  {
    std::ostringstream output;
    output << "Text=" << settings.textScalePercent
           << "% subtitles=" << (settings.subtitlesEnabled ? "on" : "off")
           << " subtitle_scale=" << settings.subtitleScalePercent
           << "% subtitle_background=" << (settings.subtitleBackgroundEnabled ? "on" : "off")
           << " sensitivity=" << settings.cameraSensitivity
           << " invert_y=" << (settings.invertY ? "true" : "false")
           << " hold_to_interact=" << (settings.holdToInteract ? "true" : "false")
           << " reduced_time_pressure=" << (settings.reducedTimePressure ? "true" : "false")
           << " aim_assist=" << (settings.lightAimAssist ? "light" : "off")
           << " high_contrast=" << (settings.highContrastEnabled ? "true" : "false")
           << " icon_redundancy=" << (settings.iconRedundancyEnabled ? "true" : "false")
           << " motion_comfort=" << (settings.motionComfortEnabled ? "true" : "false");

    for (const auto& binding : bindings)
    {
      const auto override = settings.actionBindings.find(binding.actionId);
      output << "\n- " << ResolveActionDisplayLabel(binding)
             << " [" << binding.categoryId << "]"
             << ": "
             << (override == settings.actionBindings.end() ? binding.primaryInput : override->second);
    }
    return output.str();
  }

  std::string RenderOnboardingFunnel(const std::vector<OnboardingCheckpoint>& checkpoints)
  {
    std::ostringstream output;
    output << "First 15 minutes";
    for (const auto& checkpoint : checkpoints)
    {
      output << "\n- " << LookupChildFacingText(checkpoint.labelTextId)
             << ": " << (checkpoint.completed ? "done" : "next")
             << " | help: " << LookupChildFacingText(checkpoint.helpTextId);
    }
    return output.str();
  }

  std::string RenderWorkshopTracks(
    const std::vector<Peter::Progression::UpgradeTrackDefinition>& tracks,
    const Peter::Progression::WorkshopState& workshopState)
  {
    return Peter::Progression::RenderTrackSummary(tracks, workshopState);
  }

  std::string RenderCreatorPanel(
    const std::map<std::string, double, std::less<>>& tinkerValues,
    const Peter::Workshop::LogicRulesetDefinition* activeRuleset,
    const Peter::Workshop::TinyScriptDefinition* activeScript)
  {
    std::ostringstream output;
    output << "Creator Workshop";
    output << "\nTinker values:";
    for (const auto& [variableId, value] : tinkerValues)
    {
      output << "\n- " << variableId << "=" << value;
    }
    output << "\nLogic ruleset: " << (activeRuleset == nullptr ? "none" : activeRuleset->displayName);
    output << "\nTiny script: " << (activeScript == nullptr ? "none" : activeScript->displayName);
    return output.str();
  }

  std::string RenderTinyScriptEditor(const Peter::Workshop::TinyScriptDefinition& script)
  {
    std::ostringstream output;
    output << "Tiny Script Editor\n"
           << "Hook: " << Peter::Workshop::ToString(script.hookKind) << "\n"
           << "Target action: " << (script.targetActionId.empty() ? "n/a" : script.targetActionId) << "\n"
           << script.body;
    return output.str();
  }

  std::string RenderReplaySnippet(const Peter::Workshop::CreatorReplaySnippet& snippet)
  {
    std::ostringstream output;
    output << "Creator Replay\n"
           << "Scenario: " << snippet.scenarioId << "\n"
           << "Before: " << snippet.beforeSummary << "\n"
           << "After: " << snippet.afterSummary << "\n"
           << "Change: " << snippet.changeSummary;
    for (const auto& entry : snippet.timeline)
    {
      output << "\n- " << entry;
    }
    return output.str();
  }

  std::string RenderMentorSummary(
    const Peter::Workshop::CreatorManifest& manifest,
    const Peter::Workshop::CreatorProgressState& progress,
    const Peter::AI::AgentExplainSnapshot& snapshot)
  {
    std::ostringstream output;
    output << "Mentor View\n"
           << "Active drafts:";
    for (const auto& [kindId, contentId] : manifest.activeDraftIds)
    {
      output << "\n- " << kindId << "=" << contentId;
    }
    output << "\nCompleted creator lessons=" << progress.completedCreatorLessons.size()
           << "\nMentor unlocked=" << (progress.mentorViewUnlocked ? "true" : "false")
           << "\nCurrent goal=" << snapshot.currentGoal
           << "\nTop reason=" << snapshot.topReason;
    return output.str();
  }

  std::string RenderCreatorValidationMessage(const bool valid, const std::string_view summary)
  {
    return LookupChildFacingText(valid ? "workshop.validation.valid" : "workshop.validation.invalid") +
      std::string("\n") + std::string(summary);
  }

  Peter::Core::StructuredFields ToSaveFields(const AccessibilitySettings& settings)
  {
    return Peter::Core::StructuredFields{
      {"schema_version", "2"},
      {"action_bindings", SerializeBindings(settings.actionBindings)},
      {"camera_sensitivity", std::to_string(settings.cameraSensitivity)},
      {"hold_to_interact", settings.holdToInteract ? "true" : "false"},
      {"high_contrast_enabled", settings.highContrastEnabled ? "true" : "false"},
      {"icon_redundancy_enabled", settings.iconRedundancyEnabled ? "true" : "false"},
      {"invert_y", settings.invertY ? "true" : "false"},
      {"light_aim_assist", settings.lightAimAssist ? "true" : "false"},
      {"motion_comfort_enabled", settings.motionComfortEnabled ? "true" : "false"},
      {"reduced_time_pressure", settings.reducedTimePressure ? "true" : "false"},
      {"subtitle_background_enabled", settings.subtitleBackgroundEnabled ? "true" : "false"},
      {"subtitle_scale_percent", std::to_string(settings.subtitleScalePercent)},
      {"subtitles_enabled", settings.subtitlesEnabled ? "true" : "false"},
      {"text_scale_percent", std::to_string(settings.textScalePercent)}
    };
  }

  AccessibilitySettings AccessibilitySettingsFromSaveFields(
    const Peter::Core::StructuredFields& fields,
    const std::vector<Peter::Adapters::ActionBinding>& defaults)
  {
    AccessibilitySettings settings;
    for (const auto& binding : defaults)
    {
      settings.actionBindings[binding.actionId] = binding.primaryInput;
    }

    const auto actionBindings = fields.find("action_bindings");
    const auto cameraSensitivity = fields.find("camera_sensitivity");
    const auto holdToInteract = fields.find("hold_to_interact");
    const auto highContrastEnabled = fields.find("high_contrast_enabled");
    const auto iconRedundancyEnabled = fields.find("icon_redundancy_enabled");
    const auto invertY = fields.find("invert_y");
    const auto lightAimAssist = fields.find("light_aim_assist");
    const auto motionComfortEnabled = fields.find("motion_comfort_enabled");
    const auto reducedTimePressure = fields.find("reduced_time_pressure");
    const auto subtitleBackgroundEnabled = fields.find("subtitle_background_enabled");
    const auto subtitleScale = fields.find("subtitle_scale_percent");
    const auto subtitlesEnabled = fields.find("subtitles_enabled");
    const auto textScale = fields.find("text_scale_percent");

    if (actionBindings != fields.end())
    {
      settings.actionBindings = ParseBindings(actionBindings->second);
    }
    settings.cameraSensitivity = cameraSensitivity == fields.end() ? 1.0 : std::stod(cameraSensitivity->second);
    settings.holdToInteract = holdToInteract == fields.end() ? true : holdToInteract->second == "true";
    settings.highContrastEnabled =
      highContrastEnabled != fields.end() && highContrastEnabled->second == "true";
    settings.iconRedundancyEnabled =
      iconRedundancyEnabled == fields.end() ? true : iconRedundancyEnabled->second == "true";
    settings.invertY = invertY != fields.end() && invertY->second == "true";
    settings.lightAimAssist = lightAimAssist == fields.end() ? true : lightAimAssist->second == "true";
    settings.motionComfortEnabled =
      motionComfortEnabled != fields.end() && motionComfortEnabled->second == "true";
    settings.reducedTimePressure = reducedTimePressure != fields.end() && reducedTimePressure->second == "true";
    settings.subtitleBackgroundEnabled =
      subtitleBackgroundEnabled == fields.end() ? true : subtitleBackgroundEnabled->second == "true";
    settings.subtitleScalePercent = subtitleScale == fields.end() ? 100 : std::stoi(subtitleScale->second);
    settings.subtitlesEnabled = subtitlesEnabled == fields.end() ? true : subtitlesEnabled->second == "true";
    settings.textScalePercent = textScale == fields.end() ? 100 : std::stoi(textScale->second);
    return settings;
  }
} // namespace Peter::UI
