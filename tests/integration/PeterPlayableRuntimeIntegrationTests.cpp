#include "PlayableSessionController.h"

#include "PeterAdapters/PlatformServices.h"
#include "PeterCore/EventBus.h"
#include "PeterCore/ProfileService.h"
#include "PeterCore/SaveDomainStore.h"
#include "PeterTelemetry/JsonlTelemetrySink.h"
#include "PeterTest/TestMacros.h"

#include <filesystem>

PETER_TEST_MAIN({
  const auto root = std::filesystem::temp_directory_path() / "PeterCraftPlayableRuntimeIntegration";
  [[maybe_unused]] const auto removedEntries = std::filesystem::remove_all(root);

  const Peter::Adapters::BootConfig bootConfig{
    std::filesystem::current_path(),
    root,
    true
  };

  auto platform = Peter::Adapters::CreateNullPlatformServices(bootConfig);
  Peter::Core::EventBus eventBus;
  Peter::Telemetry::JsonlTelemetrySink sink(root / "Logs" / "playable-integration-events.jsonl");
  eventBus.RegisterSink(&sink);

  Peter::Core::ProfileService profileService(*platform.save, eventBus);
  const auto profile = profileService.EnsureProfile("player.playable.integration");
  Peter::Core::SaveDomainStore saveDomainStore(profile, eventBus);

  Peter::App::PlayableSessionController controller(platform, eventBus, profile, saveDomainStore);
  const auto smokeReport = controller.Run("smoke");
  PETER_ASSERT_TRUE(smokeReport.success);
  PETER_ASSERT_EQ(std::string("scene.playable.home_base"), smokeReport.currentSceneId);
  const auto missionFields = saveDomainStore.ReadDomain("save_domain.mission_progress");
  PETER_ASSERT_EQ(std::string("mission.salvage_run.machine_silo"), missionFields.at("last_mission_id"));
  PETER_ASSERT_EQ(std::string("scene.playable.home_base"), missionFields.at("last_scene_id"));
  PETER_ASSERT_TRUE(smokeReport.transitionMs >= 0.0);
  PETER_ASSERT_TRUE(smokeReport.inputToMotionLatencyMs > 0.0);
  PETER_ASSERT_TRUE(smokeReport.interactionHitchMs > 0.0);
  PETER_ASSERT_EQ(std::string("completed"), smokeReport.extractionState);

  const auto guidedReport = controller.Run("guided_first_run");
  PETER_ASSERT_TRUE(guidedReport.lastRuleEditPreview.valid);
  PETER_ASSERT_TRUE(guidedReport.summary.find("follow-distance change") != std::string::npos);
  const auto companionFields = saveDomainStore.ReadDomain("save_domain.companion_config");
  PETER_ASSERT_TRUE(companionFields.find("follow_distance_meters") != companionFields.end());
})
