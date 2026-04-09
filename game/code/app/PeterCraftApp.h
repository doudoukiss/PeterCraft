#pragma once

#include "Phase1Slice.h"

#include "PeterAdapters/PlatformServices.h"
#include "PeterCore/EventBus.h"
#include "PeterCore/FeatureRegistry.h"
#include "PeterCore/ProfileService.h"
#include "PeterCore/SaveDomainStore.h"
#include "PeterDebug/DebugOverlay.h"
#include "PeterTelemetry/JsonlTelemetrySink.h"
#include "PeterValidation/ValidationModule.h"

#include <filesystem>
#include <string>

namespace Peter::App
{
  struct AppOptions
  {
    bool developmentMode = true;
    bool smokeTest = false;
    bool visitSettings = true;
    Peter::Adapters::RuntimeMode runtimeMode = Peter::Adapters::RuntimeMode::Headless;
    std::string profileId = "player.default";
    std::string scenario = "guided_first_run";
  };

  class PeterCraftApp
  {
  public:
    explicit PeterCraftApp(AppOptions options);
    int Run();

  private:
    std::filesystem::path ResolveRepoRoot() const;
    std::filesystem::path ResolveUserRoot() const;
    [[nodiscard]] Peter::Adapters::RuntimeDescriptor ResolveRuntimeDescriptor() const;

    AppOptions m_options;
  };
} // namespace Peter::App
