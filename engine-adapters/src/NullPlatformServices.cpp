#include "PeterAdapters/PlatformServices.h"

#include <filesystem>
#include <iostream>
#include <utility>

namespace Peter::Adapters
{
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
          {"action.interact", "E", "X", "input.interact", "interaction", true, true},
          {"action.jump", "Space", "A", "input.jump", "movement", true, false},
          {"action.sprint", "Left Shift", "Left Stick Click", "input.sprint", "movement", true, true},
          {"action.crouch", "C", "B", "input.crouch", "movement", true, true}
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
    descriptor.backendId = mode == RuntimeMode::Playable ? "playable_stub" : "null_headless";
    return descriptor;
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

    result.available = false;
    result.statusCode = "backend_unavailable";
    result.message = "Playable runtime backend unavailable until Phase 7.1.";
    if (!runtimeDescriptor.playableRuntimeEnabled)
    {
      result.message += " Build option PETERCRAFT_ENABLE_PLAYABLE_RUNTIME is OFF.";
    }
    return result;
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
    return services;
  }
} // namespace Peter::Adapters
