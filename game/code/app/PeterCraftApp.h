#pragma once

#include "PeterAdapters/PlatformServices.h"
#include "PeterCore/EventBus.h"
#include "PeterCore/FeatureRegistry.h"
#include "PeterCore/ProfileService.h"
#include "PeterDebug/DebugOverlay.h"
#include "PeterTelemetry/JsonlTelemetrySink.h"
#include "PeterTools/ScenarioHarness.h"
#include "PeterUI/MenuModel.h"
#include "PeterValidation/ValidationModule.h"
#include "PeterWorld/SceneShell.h"

#include <filesystem>
#include <string>

namespace Peter::App
{
  struct AppOptions
  {
    bool developmentMode = true;
    bool smokeTest = false;
    bool visitSettings = true;
    std::string profileId = "player.default";
    std::string sceneId = "scene.foundation.empty_shell";
  };

  class PeterCraftApp
  {
  public:
    explicit PeterCraftApp(AppOptions options);
    int Run();

  private:
    void EmitFoundationEvents();
    std::filesystem::path ResolveRepoRoot() const;
    std::filesystem::path ResolveUserRoot() const;

    AppOptions m_options;
  };
} // namespace Peter::App
