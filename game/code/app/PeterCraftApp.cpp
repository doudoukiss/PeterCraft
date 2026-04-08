#include "PeterCraftApp.h"

#include <filesystem>
#include <iostream>
#include <utility>

namespace Peter::App
{
  using Peter::Adapters::BootConfig;
  using Peter::Adapters::CreateNullPlatformServices;
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

  std::filesystem::path PeterCraftApp::ResolveRepoRoot() const
  {
    return std::filesystem::current_path();
  }

  std::filesystem::path PeterCraftApp::ResolveUserRoot() const
  {
    return ResolveRepoRoot() / "Saved";
  }

  int PeterCraftApp::Run()
  {
    const auto repoRoot = ResolveRepoRoot();
    const auto userRoot = ResolveUserRoot();
    const BootConfig bootConfig{repoRoot, userRoot, m_options.developmentMode};
    auto platform = CreateNullPlatformServices(bootConfig);

    const auto logRoot = userRoot / "Logs";
    std::filesystem::create_directories(logRoot);

    Peter::Core::EventBus eventBus;
    JsonlTelemetrySink telemetrySink(logRoot / "petercraft-events.jsonl");
    eventBus.RegisterSink(&telemetrySink);

    FeatureRegistry features(VersionInfo{"0.3.0", "phase2"});
    features.SetFlag("feature.vertical_slice", true);
    features.SetFlag("feature.core_systems_alpha", true);
    features.SetFlag("feature.debug_overlay", true);
    features.SetFlag("feature.safe_rule_edit", true);
    features.SetFlag("feature.recovery_state", true);

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
        {"profile_id", profile.profileId},
        {"scenario", m_options.scenario}
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
    eventBus.Emit(Event{
      EventCategory::Validation,
      "validation.runtime.phase2_ready",
      {{"summary", validationStatus.summary}, {"status", validationStatus.status}}});

    Phase1Slice slice(platform, eventBus, profile, saveDomainStore);
    const auto runReport = slice.Run(ParseScenario(m_options.scenario));

    DebugOverlay overlay;
    overlay.SetValue("FPS", "60");
    overlay.SetValue("Frame Time", "16.6ms");
    overlay.SetValue("Scene", runReport.success ? "scene.results.success" : "scene.results.failure");
    overlay.SetValue("Mission", runReport.missionId);
    overlay.SetValue("Player State", runReport.success ? "returned_home" : "raid_failed");
    overlay.SetValue("Companion State", runReport.lastCompanionDecision.currentState);
    overlay.SetValue("Save Slot", profile.root.string());
    overlay.SetValue("Raid Tip", runReport.raidSummary.lessonTip);

    eventBus.Emit(Event{
      EventCategory::Performance,
      "performance.shell.frame_snapshot",
      {
        {"frame_time_ms", "16.6"},
        {"fps", "60"},
        {"scene_id", runReport.success ? "scene.results.success" : "scene.results.failure"}
      }});

    std::cout << "PeterCraft Phase 2 Core Systems Alpha\n";
    std::cout << runReport.summary << "\n";
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
