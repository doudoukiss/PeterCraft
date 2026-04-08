#include "PeterAdapters/PlatformServices.h"
#include "PeterCore/EventBus.h"
#include "PeterCore/ProfileService.h"
#include "PeterTelemetry/JsonlTelemetrySink.h"
#include "PeterTest/TestMacros.h"

#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

PETER_TEST_MAIN({
  const auto root = std::filesystem::temp_directory_path() / "PeterCraftPhase0Integration";
  std::filesystem::remove_all(root);

  const Peter::Adapters::BootConfig bootConfig{
    std::filesystem::current_path(),
    root,
    true
  };

  auto platform = Peter::Adapters::CreateNullPlatformServices(bootConfig);
  Peter::Core::EventBus eventBus;
  Peter::Telemetry::JsonlTelemetrySink sink(root / "Logs" / "integration-events.jsonl");
  eventBus.RegisterSink(&sink);

  Peter::Core::ProfileService profileService(*platform.save, eventBus);
  const auto profile = profileService.EnsureProfile("player.integration");
  eventBus.Emit({Peter::Core::EventCategory::Gameplay, "gameplay.integration.complete", {}});

  PETER_ASSERT_TRUE(std::filesystem::exists(profile.root));
  PETER_ASSERT_TRUE(std::filesystem::exists(profile.saveDataRoot));
  PETER_ASSERT_TRUE(std::filesystem::exists(profile.userContentRoot));

  const auto logPath = root / "Logs" / "integration-events.jsonl";
  PETER_ASSERT_TRUE(std::filesystem::exists(logPath));

  const std::string logContents = [&logPath]() {
    std::ifstream input(logPath);
    return std::string((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
  }();

  PETER_ASSERT_TRUE(logContents.find("save_load.profile.ready") != std::string::npos);
  PETER_ASSERT_TRUE(logContents.find("gameplay.integration.complete") != std::string::npos);
})
