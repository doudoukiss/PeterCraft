#include "PeterCore/QualityProfile.h"

#include "PeterCore/StructuredStore.h"

#include <cstdlib>

namespace Peter::Core
{
  namespace
  {
    std::filesystem::path ResolveFrom(std::filesystem::path start)
    {
      if (start.empty())
      {
        return {};
      }

      if (std::filesystem::is_regular_file(start))
      {
        start = start.parent_path();
      }

      auto current = std::filesystem::absolute(start);
      while (!current.empty())
      {
        const auto candidate = current / "game" / "data" / "content" / "quality-profiles";
        if (std::filesystem::exists(candidate))
        {
          return candidate;
        }

        if (current == current.root_path())
        {
          break;
        }

        current = current.parent_path();
      }

      return {};
    }

    int ParseIntField(const StructuredFields& fields, const std::string_view key, const int fallback)
    {
      const auto iterator = fields.find(std::string(key));
      return iterator == fields.end() ? fallback : std::stoi(iterator->second);
    }

    double ParseDoubleField(const StructuredFields& fields, const std::string_view key, const double fallback)
    {
      const auto iterator = fields.find(std::string(key));
      return iterator == fields.end() ? fallback : std::stod(iterator->second);
    }

    bool ParseBoolField(const StructuredFields& fields, const std::string_view key, const bool fallback)
    {
      const auto iterator = fields.find(std::string(key));
      if (iterator == fields.end())
      {
        return fallback;
      }

      return iterator->second == "true" || iterator->second == "1" || iterator->second == "yes";
    }

    std::vector<std::string> SplitCsv(const std::string_view value)
    {
      std::vector<std::string> tokens;
      std::size_t cursor = 0;
      while (cursor < value.size())
      {
        const auto separator = value.find(',', cursor);
        const auto token = value.substr(
          cursor,
          separator == std::string_view::npos ? value.size() - cursor : separator - cursor);
        if (!token.empty())
        {
          tokens.emplace_back(token);
        }

        if (separator == std::string_view::npos)
        {
          break;
        }

        cursor = separator + 1;
      }

      return tokens;
    }

    std::map<std::string, int, std::less<>> ParsePriorityMap(const std::string_view value)
    {
      std::map<std::string, int, std::less<>> priorities;
      for (const auto& token : SplitCsv(value))
      {
        const auto separator = token.find(':');
        if (separator == std::string::npos)
        {
          continue;
        }

        priorities[token.substr(0, separator)] = std::stoi(token.substr(separator + 1));
      }

      return priorities;
    }

    void PopulateDefaultFeedback(QualityProfileBase& profile)
    {
      profile.feedback.requiredCueFamilies = {
        "loot_reward",
        "extraction",
        "threat_escalation",
        "companion_ack",
        "workshop_success",
        "creator_error",
        "creator_success"
      };
      profile.feedback.cuePriorityByFamily = {
        {"loot_reward", 2},
        {"extraction", 3},
        {"threat_escalation", 3},
        {"companion_ack", 1},
        {"workshop_success", 2},
        {"creator_error", 3},
        {"creator_success", 2}
      };
    }

    void ApplyCommonFields(QualityProfileBase& profile, const StructuredFields& fields)
    {
      if (const auto iterator = fields.find("id"); iterator != fields.end())
      {
        profile.id = iterator->second;
      }
      if (const auto iterator = fields.find("display_name"); iterator != fields.end())
      {
        profile.displayName = iterator->second;
      }
      if (const auto iterator = fields.find("hardware_profile_id"); iterator != fields.end())
      {
        profile.targetHardware.id = iterator->second;
      }
      if (const auto iterator = fields.find("target_os"); iterator != fields.end())
      {
        profile.targetHardware.operatingSystem = iterator->second;
      }
      if (const auto iterator = fields.find("cpu_class"); iterator != fields.end())
      {
        profile.targetHardware.cpuClass = iterator->second;
      }
      profile.targetHardware.memoryGigabytes =
        ParseIntField(fields, "memory_gb", profile.targetHardware.memoryGigabytes);
      if (const auto iterator = fields.find("storage_class"); iterator != fields.end())
      {
        profile.targetHardware.storageClass = iterator->second;
      }
      if (const auto iterator = fields.find("gpu_class"); iterator != fields.end())
      {
        profile.targetHardware.gpuClass = iterator->second;
      }

      profile.budgets.fpsTarget = ParseIntField(fields, "fps_target", profile.budgets.fpsTarget);
      profile.budgets.frameTimeP95Ms =
        ParseDoubleField(fields, "frame_time_p95_ms", profile.budgets.frameTimeP95Ms);
      profile.budgets.coldBootBudgetMs =
        ParseDoubleField(fields, "cold_boot_budget_ms", profile.budgets.coldBootBudgetMs);
      profile.budgets.transitionBudgetMs =
        ParseDoubleField(fields, "transition_budget_ms", profile.budgets.transitionBudgetMs);
      profile.budgets.peakWorkingSetBudgetMb =
        ParseIntField(fields, "peak_working_set_budget_mb", profile.budgets.peakWorkingSetBudgetMb);
      profile.budgets.inputToMotionLatencyBudgetMs =
        ParseDoubleField(fields, "input_to_motion_latency_budget_ms", profile.budgets.inputToMotionLatencyBudgetMs);
      profile.budgets.aiDecisionP95Ms =
        ParseDoubleField(fields, "ai_decision_p95_ms", profile.budgets.aiDecisionP95Ms);
      profile.budgets.uiPanelRenderP95Ms =
        ParseDoubleField(fields, "ui_panel_render_p95_ms", profile.budgets.uiPanelRenderP95Ms);
      profile.budgets.interactionHitchBudgetMs =
        ParseDoubleField(fields, "interaction_hitch_budget_ms", profile.budgets.interactionHitchBudgetMs);
      profile.budgets.singleSaveBudgetMs =
        ParseDoubleField(fields, "single_save_budget_ms", profile.budgets.singleSaveBudgetMs);
      profile.budgets.fullLoadBudgetMs =
        ParseDoubleField(fields, "full_load_budget_ms", profile.budgets.fullLoadBudgetMs);
      profile.budgets.fullSaveBudgetMs =
        ParseDoubleField(fields, "full_save_budget_ms", profile.budgets.fullSaveBudgetMs);
      profile.budgets.audioVoiceConcurrencyBudget =
        ParseIntField(fields, "audio_voice_concurrency_budget", profile.budgets.audioVoiceConcurrencyBudget);

      profile.movement.accelerationMetersPerSecondSquared =
        ParseDoubleField(fields, "movement_acceleration_mps2", profile.movement.accelerationMetersPerSecondSquared);
      profile.movement.decelerationMetersPerSecondSquared =
        ParseDoubleField(fields, "movement_deceleration_mps2", profile.movement.decelerationMetersPerSecondSquared);
      profile.movement.jumpReadabilityScale =
        ParseDoubleField(fields, "movement_jump_readability_scale", profile.movement.jumpReadabilityScale);
      profile.movement.jumpBufferMilliseconds =
        ParseIntField(fields, "movement_jump_buffer_ms", profile.movement.jumpBufferMilliseconds);
      profile.movement.ledgeForgivenessMilliseconds =
        ParseIntField(fields, "movement_ledge_forgiveness_ms", profile.movement.ledgeForgivenessMilliseconds);
      profile.movement.companionSpacingMeters =
        ParseDoubleField(fields, "movement_companion_spacing_m", profile.movement.companionSpacingMeters);

      profile.camera.smoothingStrength =
        ParseDoubleField(fields, "camera_smoothing_strength", profile.camera.smoothingStrength);
      profile.camera.sensitivityCurveExponent =
        ParseDoubleField(fields, "camera_sensitivity_curve_exponent", profile.camera.sensitivityCurveExponent);
      profile.camera.shoulderOffsetMeters =
        ParseDoubleField(fields, "camera_shoulder_offset_m", profile.camera.shoulderOffsetMeters);
      profile.camera.followDistanceMeters =
        ParseDoubleField(fields, "camera_follow_distance_m", profile.camera.followDistanceMeters);
      profile.camera.pitchDegrees =
        ParseDoubleField(fields, "camera_pitch_degrees", profile.camera.pitchDegrees);
      profile.camera.motionComfortEnabled =
        ParseBoolField(fields, "camera_motion_comfort_enabled", profile.camera.motionComfortEnabled);

      profile.combat.telegraphWindowMilliseconds =
        ParseIntField(fields, "combat_telegraph_window_ms", profile.combat.telegraphWindowMilliseconds);
      profile.combat.hitStopMilliseconds =
        ParseIntField(fields, "combat_hit_stop_ms", profile.combat.hitStopMilliseconds);
      profile.combat.lowHealthThreshold =
        ParseIntField(fields, "combat_low_health_threshold", profile.combat.lowHealthThreshold);
      profile.combat.lineOfFireWidthMeters =
        ParseDoubleField(fields, "combat_line_of_fire_width_m", profile.combat.lineOfFireWidthMeters);
      if (const auto iterator = fields.find("combat_status_clarity_mode"); iterator != fields.end())
      {
        profile.combat.statusClarityMode = iterator->second;
      }

      profile.feedback.maxConcurrentWorldCues =
        ParseIntField(fields, "feedback_max_concurrent_world_cues", profile.feedback.maxConcurrentWorldCues);
      profile.feedback.maxCriticalCuesPerBeat =
        ParseIntField(fields, "feedback_max_critical_cues_per_beat", profile.feedback.maxCriticalCuesPerBeat);
      if (const auto iterator = fields.find("feedback_required_families"); iterator != fields.end())
      {
        profile.feedback.requiredCueFamilies = SplitCsv(iterator->second);
      }
      if (const auto iterator = fields.find("feedback_priorities"); iterator != fields.end())
      {
        profile.feedback.cuePriorityByFamily = ParsePriorityMap(iterator->second);
      }

      profile.subtitleBackgroundRequired =
        ParseBoolField(fields, "subtitle_background_required", profile.subtitleBackgroundRequired);
      profile.highContrastRequired =
        ParseBoolField(fields, "high_contrast_required", profile.highContrastRequired);
      profile.iconRedundancyRequired =
        ParseBoolField(fields, "icon_redundancy_required", profile.iconRedundancyRequired);
      profile.motionComfortRequired =
        ParseBoolField(fields, "motion_comfort_required", profile.motionComfortRequired);
    }

    template <typename TProfile>
    TProfile LoadQualityProfileFromFile(TProfile profile, const std::filesystem::path& path)
    {
      const auto parseResult = ParseStructuredFieldsFile(path);
      if (!parseResult.valid || parseResult.fields.empty())
      {
        return profile;
      }

      ApplyCommonFields(profile, parseResult.fields);
      return profile;
    }
  } // namespace

  std::filesystem::path ResolveQualityProfileRoot()
  {
    std::string envRoot;
#ifdef _WIN32
    char* buffer = nullptr;
    std::size_t size = 0;
    if (_dupenv_s(&buffer, &size, "PETERCRAFT_REPO_ROOT") == 0 && buffer != nullptr)
    {
      envRoot = buffer;
      free(buffer);
    }
#else
    if (const char* rawEnvRoot = std::getenv("PETERCRAFT_REPO_ROOT"))
    {
      envRoot = rawEnvRoot;
    }
#endif

    if (!envRoot.empty())
    {
      const auto path = ResolveFrom(envRoot);
      if (!path.empty())
      {
        return path;
      }
    }

    return ResolveFrom(std::filesystem::current_path());
  }

  Phase6QualityProfile BuildDefaultPhase6QualityProfile()
  {
    Phase6QualityProfile profile;
    profile.id = "quality.phase6.shell";
    profile.displayName = "Phase 6 Shell Quality Profile";
    profile.targetHardware = {
      "hardware.win11.midrange",
      "Windows 11",
      "6-core desktop CPU",
      16,
      "SSD",
      "GTX 1660 / RX 580 class"
    };
    PopulateDefaultFeedback(profile);
    return profile;
  }

  Phase6QualityProfile LoadPhase6QualityProfile()
  {
    return LoadQualityProfileFromFile(
      BuildDefaultPhase6QualityProfile(),
      ResolveQualityProfileRoot() / "quality.phase6.shell.json");
  }

  Phase7PlayableQualityProfile BuildDefaultPhase7PlayableQualityProfile()
  {
    Phase7PlayableQualityProfile profile;
    profile.id = "quality.phase7.playable";
    profile.displayName = "Phase 7 Playable Runtime Quality Profile";
    profile.targetHardware = {
      "hardware.win11.midrange",
      "Windows 11",
      "6-core desktop CPU",
      16,
      "SSD",
      "GTX 1660 / RX 580 class"
    };
    profile.budgets.inputToMotionLatencyBudgetMs = 85.0;
    profile.budgets.interactionHitchBudgetMs = 50.0;
    profile.budgets.audioVoiceConcurrencyBudget = 16;
    PopulateDefaultFeedback(profile);
    return profile;
  }

  Phase7PlayableQualityProfile LoadPhase7PlayableQualityProfile()
  {
    return LoadQualityProfileFromFile(
      BuildDefaultPhase7PlayableQualityProfile(),
      ResolveQualityProfileRoot() / "quality.phase7.playable.json");
  }

  std::string DescribeTargetHardwareProfile(const TargetHardwareProfile& profile)
  {
    return profile.operatingSystem + " | " + profile.cpuClass + " | " +
      std::to_string(profile.memoryGigabytes) + " GB | " + profile.storageClass + " | " + profile.gpuClass;
  }
} // namespace Peter::Core
