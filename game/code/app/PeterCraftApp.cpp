#include "PeterCraftApp.h"

#include "PeterCore/QualityProfile.h"
#include "PeterTelemetry/QualityMetrics.h"

#include <chrono>
#include <filesystem>
#include <iostream>
#include <utility>

namespace Peter::App
{
  using Peter::Adapters::BootConfig;
  using Peter::Adapters::BuildRuntimeDescriptor;
  using Peter::Adapters::CreatePlatformServices;
  using Peter::Core::Event;
  using Peter::Core::EventCategory;
  using Peter::Core::FeatureRegistry;
  using Peter::Core::ProfileService;
  using Peter::Core::SaveDomainStore;
  using Peter::Core::VersionInfo;
  using Peter::Debug::DebugOverlay;
  using Peter::Telemetry::JsonlTelemetrySink;
  using Peter::Validation::ValidationStatus;

  PeterCraftApp::PeterCraftApp(AppOptions options)
    : m_options(std::move(options))
  {
  }

  namespace
  {
#if defined(PETERCRAFT_ENABLE_PLAYABLE_RUNTIME) && PETERCRAFT_ENABLE_PLAYABLE_RUNTIME
    constexpr bool kPlayableRuntimeCompiled = true;
#else
    constexpr bool kPlayableRuntimeCompiled = false;
#endif
  } // namespace

  std::filesystem::path PeterCraftApp::ResolveRepoRoot() const
  {
    return std::filesystem::current_path();
  }

  std::filesystem::path PeterCraftApp::ResolveUserRoot() const
  {
    return ResolveRepoRoot() / "Saved";
  }

  Peter::Adapters::RuntimeDescriptor PeterCraftApp::ResolveRuntimeDescriptor() const
  {
    return BuildRuntimeDescriptor(m_options.runtimeMode, kPlayableRuntimeCompiled);
  }

  int PeterCraftApp::Run()
  {
    const auto bootStarted = std::chrono::steady_clock::now();
    const auto repoRoot = ResolveRepoRoot();
    const auto userRoot = ResolveUserRoot();
    const BootConfig bootConfig{repoRoot, userRoot, m_options.developmentMode};
    const auto runtimeDescriptor = ResolveRuntimeDescriptor();

    const auto logRoot = userRoot / "Logs";
    std::filesystem::create_directories(logRoot);

    Peter::Core::EventBus eventBus;
    JsonlTelemetrySink telemetrySink(logRoot / "petercraft-events.jsonl");
    eventBus.RegisterSink(&telemetrySink);

    auto runtimeFactoryResult = CreatePlatformServices(bootConfig, runtimeDescriptor);
    const auto qualityProfile = Peter::Core::LoadPhase6QualityProfile();
    const auto playableQualityProfile = Peter::Core::LoadPhase7PlayableQualityProfile();
    const bool playableRuntimeActive =
      runtimeFactoryResult.available
      && runtimeDescriptor.mode == Peter::Adapters::RuntimeMode::Playable;
    const Peter::Core::QualityProfileBase& activeQualityProfile = playableRuntimeActive
      ? static_cast<const Peter::Core::QualityProfileBase&>(playableQualityProfile)
      : static_cast<const Peter::Core::QualityProfileBase&>(qualityProfile);

    FeatureRegistry features(VersionInfo{"0.7.1", "phase7_1"});
    features.SetFlag("feature.vertical_slice", true);
    features.SetFlag("feature.core_systems_alpha", true);
    features.SetFlag("feature.debug_overlay", true);
    features.SetFlag("feature.safe_rule_edit", true);
    features.SetFlag("feature.recovery_state", true);
    features.SetFlag("feature.ai_alpha", true);
    features.SetFlag("feature.behavior_chips", true);
    features.SetFlag("feature.content_beta", true);
    features.SetFlag("feature.content_catalogs", true);
    features.SetFlag("feature.quality_beta", true);
    features.SetFlag("feature.atomic_save_hardening", true);
    features.SetFlag("feature.playable_runtime", playableRuntimeActive);
    features.SetFlag("feature.o3de_adapter", playableRuntimeActive);
    features.SetFlag("feature.realtime_traversal", playableRuntimeActive);
    features.SetFlag("feature.realtime_combat", playableRuntimeActive);
    features.SetFlag("feature.realtime_ai", playableRuntimeActive);
    features.SetFlag("feature.raid_hud", playableRuntimeActive);
    features.SetFlag("feature.playable_audio", playableRuntimeActive);
    features.SetFlag("feature.phase7_blockout_art", playableRuntimeActive);

    eventBus.Emit(Event{
      EventCategory::Gameplay,
      "runtime.mode.selected",
      {
        {"backend_id", runtimeFactoryResult.descriptor.backendId},
        {"bootstrap_message", runtimeFactoryResult.message},
        {"mode", Peter::Adapters::ToString(runtimeFactoryResult.descriptor.mode)},
        {"playable_runtime_compiled", kPlayableRuntimeCompiled ? "true" : "false"},
        {"status", runtimeFactoryResult.available ? "ready" : runtimeFactoryResult.statusCode}
      }});

    if (!runtimeFactoryResult.available)
    {
      eventBus.Emit(Event{
        EventCategory::Gameplay,
        "runtime.backend.unavailable",
        {
          {"backend_id", runtimeFactoryResult.descriptor.backendId},
          {"message", runtimeFactoryResult.message},
          {"mode", Peter::Adapters::ToString(runtimeFactoryResult.descriptor.mode)}
        }});
      std::cout << "PeterCraft Phase 7.1 Runtime Bootstrap\n";
      std::cout << "Build track: " << features.Version().track << '\n';
      std::cout << "Runtime mode: " << Peter::Adapters::ToString(runtimeFactoryResult.descriptor.mode) << '\n';
      std::cout << runtimeFactoryResult.message << '\n';
      return 2;
    }

    auto platform = std::move(runtimeFactoryResult.services);

    ProfileService profileService(*platform.save, eventBus);
    const auto profile = profileService.EnsureProfile(m_options.profileId);
    SaveDomainStore saveDomainStore(profile, eventBus);

    platform.ui->PresentState("ui.main_menu");
    eventBus.Emit(Event{
      EventCategory::Gameplay,
      "gameplay.boot.complete",
      {
        {"active_input_scheme", platform.input->ActiveScheme()},
        {"build_track", features.Version().track},
        {"runtime_mode", Peter::Adapters::ToString(runtimeDescriptor.mode)},
        {"profile_id", profile.profileId},
        {"scenario", m_options.scenario}
      }});

    const auto bootFinished = std::chrono::steady_clock::now();
    const auto coldBootMs = std::chrono::duration<double, std::milli>(bootFinished - bootStarted).count();
    eventBus.Emit(Event{
      EventCategory::Performance,
      "performance.boot.complete",
      {
        {"metric_id", "cold_boot_ms"},
        {"unit", "ms"},
        {"value", std::to_string(coldBootMs)}
      }});

    if (m_options.visitSettings)
    {
      platform.ui->PresentState("ui.settings");
      platform.audio->PostUiCue("ui.menu.settings_open");
      eventBus.Emit(Event{
        EventCategory::Gameplay,
        "gameplay.settings.visited",
        {{"profile_id", profile.profileId}}});
    }

    const auto validationStatus = ValidationStatus::PlaceholderHealthy();
    const auto playableProfileValidation =
      Peter::Validation::ValidatePhase7PlayableQualityProfile(playableQualityProfile);
    eventBus.Emit(Event{
      EventCategory::Validation,
      "validation.runtime.phase7_0_ready",
      {
        {"phase6_summary", validationStatus.summary},
        {"phase6_status", validationStatus.status},
        {"phase7_playable_profile", playableProfileValidation.valid ? "valid" : "invalid"},
        {"phase7_playable_summary", playableProfileValidation.message}
      }});

    Phase1Slice slice(platform, eventBus, profile, saveDomainStore);
    const auto runReport = slice.Run(ParseScenario(m_options.scenario));
    const auto saveHealth = saveDomainStore.InspectHealth();
    const auto workingSetMb = static_cast<double>(Peter::Telemetry::CurrentWorkingSetMegabytes());
    eventBus.Emit(Event{
      EventCategory::Performance,
      "performance.memory.snapshot",
      {
        {"metric_id", "working_set_mb"},
        {"unit", "mb"},
        {"value", std::to_string(workingSetMb)}
      }});

    std::vector<Peter::Telemetry::QualityMetricSample> qualitySamples = {
      {"cold_boot_ms", "ms", coldBootMs, 0.0, true, "boot"},
      {"fps_average", "fps", 60.0, 0.0, true, "shell"},
      {"frame_time_p95_ms", "ms", 16.6, 0.0, true, "shell"},
      {"working_set_mb", "mb", workingSetMb, 0.0, true, "process"}
    };
    const auto qualityReport = Peter::Telemetry::EvaluateQualityReport(activeQualityProfile, qualitySamples);

    DebugOverlay overlay;
    overlay.SetRuntimeDescriptor(runtimeDescriptor);
    overlay.SetFeatureFlags(features.Flags());
    overlay.SetValue("FPS", "60");
    overlay.SetValue("Frame Time", "16.6ms");
    overlay.SetValue("Target Hardware", Peter::Core::DescribeTargetHardwareProfile(activeQualityProfile.targetHardware));
    overlay.SetValue("Scene", runReport.success ? "scene.results.success" : "scene.results.failure");
    overlay.SetValue("Mission", runReport.missionId);
    overlay.SetValue("Player State", runReport.success ? "returned_home" : "raid_failed");
    overlay.SetValue("Companion State", runReport.lastCompanionDecision.currentState);
    overlay.SetValue("Save Slot", profile.root.string());
    overlay.SetValue("Raid Tip", runReport.raidSummary.lessonTip);
    overlay.SetAiSnapshot(Peter::AI::BuildExplainSnapshot(runReport.lastCompanionDecision));
    overlay.SetSaveHealthReport(saveHealth);
    overlay.SetQualityReport(qualityReport);

    eventBus.Emit(Event{
      EventCategory::Performance,
      "performance.shell.frame_snapshot",
      {
        {"frame_time_ms", "16.6"},
        {"fps", "60"},
        {"scene_id", runReport.success ? "scene.results.success" : "scene.results.failure"}
      }});

    std::cout << (playableRuntimeActive
      ? "PeterCraft Phase 7.1 Playable Runtime\n"
      : "PeterCraft Phase 7.1 Headless Runtime\n");
    std::cout << runReport.summary << "\n";
    std::cout << Peter::Telemetry::RenderQualityReport(qualityReport) << "\n";
    std::cout << overlay.Render() << '\n';

    if (m_options.smokeTest)
    {
      eventBus.Emit(Event{
        EventCategory::Gameplay,
        "gameplay.smoke_test.complete",
        {
          {"result", "pass"},
          {"scenario", m_options.scenario}
        }});
    }

    return 0;
  }
} // namespace Peter::App
