#include "PeterCraftApp.h"

#include "PeterAI/AiModule.h"

#include <cstdlib>
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
  using Peter::Core::StructuredFields;
  using Peter::Core::VersionInfo;
  using Peter::Debug::DebugOverlay;
  using Peter::Telemetry::JsonlTelemetrySink;
  using Peter::Tools::DeterministicScenarioHarness;
  using Peter::UI::MenuModel;
  using Peter::Validation::ValidationStatus;
  using Peter::World::SceneShell;

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

  void PeterCraftApp::EmitFoundationEvents()
  {
    (void)this;
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

    FeatureRegistry features(VersionInfo{"0.1.0", "phase0"});
    features.SetFlag("feature.empty_scene_shell", true);
    features.SetFlag("feature.debug_overlay", true);
    features.SetFlag("feature.creator_mode_locked", true);

    ProfileService profileService(*platform.save, eventBus);
    const auto profile = profileService.EnsureProfile(m_options.profileId);

    MenuModel menuModel;
    menuModel.EnterMainMenu();
    platform.ui->PresentState(menuModel.ActiveState());

    eventBus.Emit(Event{
      EventCategory::Gameplay,
      "gameplay.boot.complete",
      {
        {"active_input_scheme", platform.input->ActiveScheme()},
        {"build_track", features.Version().track},
        {"profile_id", profile.profileId},
        {"scene_target", m_options.sceneId}
      }});

    if (m_options.visitSettings)
    {
      menuModel.EnterSettings();
      platform.ui->PresentState(menuModel.ActiveState());
      platform.audio->PostUiCue("ui.menu.settings_open");
      eventBus.Emit(Event{
        EventCategory::Gameplay,
        "gameplay.settings.visited",
        {{"profile_id", profile.profileId}}});
    }

    const auto validationStatus = ValidationStatus::PlaceholderHealthy();
    eventBus.Emit(Event{
      EventCategory::Validation,
      "validation.runtime.placeholder_ready",
      {{"summary", validationStatus.summary}, {"status", validationStatus.status}}});

    eventBus.Emit(Event{
      EventCategory::CreatorTools,
      "creator_tools.locked_for_phase0",
      {{"reason", "foundation_shell_only"}}});

    eventBus.Emit(Event{
      EventCategory::AI,
      "ai.companion.placeholder_state",
      {
        {"module_summary", std::string(Peter::AI::GetModuleSummary())},
        {"state", "idle_placeholder"}
      }});

    SceneShell sceneShell(eventBus);
    const auto sceneState = sceneShell.LoadScene(m_options.sceneId);

    eventBus.Emit(Event{
      EventCategory::Performance,
      "performance.shell.frame_snapshot",
      {
        {"frame_time_ms", "16.6"},
        {"fps", "60"},
        {"scene_id", sceneState.sceneId}
      }});

    DeterministicScenarioHarness scenarioHarness("scenario.foundation.shell_boot");
    const auto scenarioReport = scenarioHarness.Run({"menu_boot", "settings_visit", "scene_load"});
    eventBus.Emit(Event{
      EventCategory::Gameplay,
      "gameplay.scenario.recorded",
      {
        {"scenario_id", scenarioReport.scenarioId},
        {"step_count", std::to_string(scenarioReport.stepCount)}
      }});

    DebugOverlay overlay;
    overlay.SetValue("FPS", "60");
    overlay.SetValue("Frame Time", "16.6ms");
    overlay.SetValue("Scene", sceneState.sceneId);
    overlay.SetValue("Mission", "mission.foundation.none");
    overlay.SetValue("Player State", "boot_shell_ready");
    overlay.SetValue("Companion State", "placeholder_idle");
    overlay.SetValue("Save Slot", profile.root.string());

    std::cout << "PeterCraft Phase 0 Shell\n";
    std::cout << overlay.Render() << '\n';

    if (m_options.smokeTest)
    {
      eventBus.Emit(Event{
        EventCategory::Gameplay,
        "gameplay.smoke_test.complete",
        {{"result", "pass"}}});
    }

    return 0;
  }
} // namespace Peter::App
