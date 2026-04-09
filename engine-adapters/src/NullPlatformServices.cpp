#include "PeterAdapters/PlatformServices.h"

#include <filesystem>
#include <cstdlib>
#include <iostream>
#include <utility>

namespace Peter::Adapters
{
#if defined(PETERCRAFT_ENABLE_PLAYABLE_RUNTIME) && PETERCRAFT_ENABLE_PLAYABLE_RUNTIME && defined(_WIN32)
  PlatformFactoryResult CreateO3DEPlatformServices(
    const BootConfig& bootConfig,
    const RuntimeDescriptor& runtimeDescriptor);
  O3DEBootstrapResult BootstrapO3DEProjectImpl(const O3DEBootstrapConfig& config);
#endif

  namespace
  {
    class NullInputAdapter final : public IInputAdapter
    {
    public:
      std::string ActiveScheme() const override
      {
        return "mouse_keyboard";
      }

      InputState SampleInput() const override
      {
        return {};
      }

      std::vector<ActionBinding> DefaultBindings() const override
      {
        return {
          {"action.move", "WASD", "Left Stick", "input.move", "movement", false, false},
          {"action.look", "Mouse", "Right Stick", "input.look", "camera", false, false},
          {"action.interact", "E", "X", "input.interact", "interaction", true, true},
          {"action.primary_action", "Left Mouse", "Right Trigger", "input.primary_action", "combat", true, true},
          {"action.secondary_action", "Right Mouse", "Left Trigger", "input.secondary_action", "combat", true, true},
          {"action.jump", "Space", "A", "input.jump", "movement", true, false},
          {"action.sprint", "Left Shift", "Left Stick Click", "input.sprint", "movement", true, true},
          {"action.crouch", "C", "B", "input.crouch", "movement", true, true},
          {"action.call_companion", "Q", "Y", "input.call_companion", "companion", true, false},
          {"action.open_inventory", "Tab", "View", "input.open_inventory", "menu", true, false},
          {"action.open_explain", "F", "DPad Up", "input.open_explain", "menu", true, false},
          {"action.pause", "Escape", "Menu", "input.pause", "menu", true, false}
        };
      }
    };

    class NullCameraAdapter final : public ICameraAdapter
    {
    public:
      CameraRigState CurrentRig() const override
      {
        return m_state;
      }

      void ApplyRig(const CameraRigState& state) override
      {
        m_state = state;
        std::cout << "[camera] mode=" << m_state.mode << " follow_distance="
                  << m_state.followDistanceMeters << '\n';
      }

    private:
      CameraRigState m_state;
    };

    class NullSaveAdapter final : public ISaveAdapter
    {
    public:
      explicit NullSaveAdapter(std::filesystem::path root)
        : m_root(std::move(root))
      {
      }

      std::filesystem::path ResolveProfileRoot() const override
      {
        return m_root / "Profiles";
      }

      void EnsureDirectory(const std::filesystem::path& directory) const override
      {
        std::filesystem::create_directories(directory);
      }

    private:
      std::filesystem::path m_root;
    };

    class NullNavigationAdapter final : public INavigationAdapter
    {
    public:
      std::string BackendName() const override
      {
        return "null_navmesh";
      }

      std::vector<std::string> ResolvePath(
        const std::string_view fromNodeId,
        const std::string_view toNodeId) const override
      {
        return {std::string(fromNodeId), std::string(toNodeId)};
      }
    };

    class NullAudioAdapter final : public IAudioAdapter
    {
    public:
      void PostUiCue(std::string_view cueId) override
      {
        std::cout << "[audio] " << cueId << '\n';
      }

      void PostWorldCue(std::string_view cueId) override
      {
        std::cout << "[world-audio] " << cueId << '\n';
      }

      void PostFeedbackCue(std::string_view cueFamily, std::string_view variantId) override
      {
        std::cout << "[feedback-audio] " << cueFamily << ':' << variantId << '\n';
      }

      void PostPrioritizedFeedbackCue(
        std::string_view cueFamily,
        std::string_view variantId,
        const int priority,
        const bool critical) override
      {
        std::cout << "[feedback-audio] " << cueFamily << ':' << variantId
                  << " priority=" << priority
                  << " critical=" << (critical ? "true" : "false") << '\n';
      }
    };

    class NullUiAdapter final : public IUiAdapter
    {
    public:
      void PresentState(std::string_view stateId) override
      {
        std::cout << "[ui] " << stateId << '\n';
      }

      void PresentPrompt(std::string_view prompt) override
      {
        std::cout << "[prompt] " << prompt << '\n';
      }

      void PresentPanel(std::string_view panelId, std::string_view body) override
      {
        std::cout << "[panel:" << panelId << "] " << body << '\n';
      }

      void PresentCreatorPanel(std::string_view panelId, std::string_view body) override
      {
        std::cout << "[creator-panel:" << panelId << "] " << body << '\n';
      }

      void PresentTextEditor(std::string_view panelId, std::string_view body) override
      {
        std::cout << "[text-editor:" << panelId << "] " << body << '\n';
      }

      void PresentReplayTimeline(std::string_view panelId, std::string_view body) override
      {
        std::cout << "[replay:" << panelId << "] " << body << '\n';
      }

      void PresentMentorSummaryPrompt(std::string_view exportPath, std::string_view body) override
      {
        std::cout << "[mentor-summary] path=" << exportPath << ' ' << body << '\n';
      }

      void ApplyPresentationSettings(const PresentationSettings& settings) override
      {
        std::cout << "[ui-settings] subtitles=" << (settings.subtitlesEnabled ? "on" : "off")
                  << " subtitle_scale=" << settings.subtitleScalePercent
                  << " text_scale=" << settings.textScalePercent
                  << " subtitle_background=" << (settings.subtitleBackgroundEnabled ? "on" : "off")
                  << " high_contrast=" << (settings.highContrastEnabled ? "true" : "false")
                  << " icon_redundancy=" << (settings.iconRedundancyEnabled ? "true" : "false")
                  << " motion_comfort=" << (settings.motionComfortEnabled ? "true" : "false") << '\n';
      }

      void PresentDebugMarkers(const std::vector<std::string>& markerIds) override
      {
        std::cout << "[debug-markers]";
        for (const auto& markerId : markerIds)
        {
          std::cout << ' ' << markerId;
        }
        std::cout << '\n';
      }

      void PresentCompanionFeedback(
        const std::string_view calloutToken,
        const std::string_view gestureToken) override
      {
        std::cout << "[companion-feedback] callout=" << calloutToken
                  << " gesture=" << gestureToken << '\n';
      }
    };

    class NullSceneAdapter final : public ISceneAdapter
    {
    public:
      std::string BackendName() const override
      {
        return "headless_scene_shell";
      }

      SceneLoadResult LoadScene(const SceneLoadRequest& request) override
      {
        return {
          true,
          BackendName(),
          "headless_noop",
          "Headless runtime acknowledged logical scene '" + request.logicalSceneId + "'."};
      }
    };
  } // namespace

  std::string ToString(const RuntimeMode mode)
  {
    return mode == RuntimeMode::Playable ? "playable" : "headless";
  }

  bool TryParseRuntimeMode(const std::string_view value, RuntimeMode& mode)
  {
    if (value == "headless")
    {
      mode = RuntimeMode::Headless;
      return true;
    }

    if (value == "playable")
    {
      mode = RuntimeMode::Playable;
      return true;
    }

    return false;
  }

  RuntimeDescriptor BuildRuntimeDescriptor(
    const RuntimeMode mode,
    const bool playableRuntimeEnabled)
  {
    RuntimeDescriptor descriptor;
    descriptor.mode = mode;
    descriptor.playableRuntimeEnabled = playableRuntimeEnabled;
    descriptor.backendId = mode == RuntimeMode::Playable
      ? (playableRuntimeEnabled ? "o3de_playable" : "playable_stub")
      : "null_headless";
    return descriptor;
  }

  std::filesystem::path ResolveDefaultO3DERoot()
  {
    char* configuredRoot = nullptr;
    std::size_t configuredRootLength = 0;
    if (_dupenv_s(&configuredRoot, &configuredRootLength, "PETERCRAFT_O3DE_ROOT") == 0
      && configuredRoot != nullptr
      && configuredRootLength > 1)
    {
      const std::filesystem::path root(configuredRoot);
      std::free(configuredRoot);
      return root;
    }
    std::free(configuredRoot);

    return std::filesystem::path("C:/o3de/25.10.2");
  }

  O3DEBootstrapResult BootstrapO3DEProject(const O3DEBootstrapConfig& config)
  {
#if defined(PETERCRAFT_ENABLE_PLAYABLE_RUNTIME) && PETERCRAFT_ENABLE_PLAYABLE_RUNTIME && defined(_WIN32)
    return BootstrapO3DEProjectImpl(config);
#else
    O3DEBootstrapResult result;
    result.success = false;
    result.statusCode = "playable_runtime_not_compiled";
    result.message = "Playable runtime bootstrap is unavailable in this build.";
    result.engineRoot = config.engineRoot;
    result.projectRoot = config.projectRoot;
    return result;
#endif
  }

  PlatformFactoryResult CreatePlatformServices(
    const BootConfig& bootConfig,
    const RuntimeDescriptor& runtimeDescriptor)
  {
    PlatformFactoryResult result;
    result.descriptor = runtimeDescriptor;

    if (runtimeDescriptor.mode == RuntimeMode::Headless)
    {
      result.services = CreateNullPlatformServices(bootConfig);
      return result;
    }

    if (!runtimeDescriptor.playableRuntimeEnabled)
    {
      result.available = false;
      result.statusCode = "playable_runtime_not_compiled";
      result.message = "Playable runtime is disabled in this build. Rebuild with PETERCRAFT_ENABLE_PLAYABLE_RUNTIME=ON.";
      return result;
    }

#if defined(PETERCRAFT_ENABLE_PLAYABLE_RUNTIME) && PETERCRAFT_ENABLE_PLAYABLE_RUNTIME && defined(_WIN32)
    return CreateO3DEPlatformServices(bootConfig, runtimeDescriptor);
#else
    result.available = false;
    result.statusCode = "backend_unavailable";
    result.message = "Playable runtime requires a Windows build with O3DE runtime support enabled.";
    return result;
#endif
  }

  PlatformServices CreateNullPlatformServices(const BootConfig& bootConfig)
  {
    PlatformServices services;
    services.input = std::make_unique<NullInputAdapter>();
    services.camera = std::make_unique<NullCameraAdapter>();
    services.save = std::make_unique<NullSaveAdapter>(bootConfig.userRoot);
    services.navigation = std::make_unique<NullNavigationAdapter>();
    services.audio = std::make_unique<NullAudioAdapter>();
    services.ui = std::make_unique<NullUiAdapter>();
    services.scene = std::make_unique<NullSceneAdapter>();
    return services;
  }
} // namespace Peter::Adapters
