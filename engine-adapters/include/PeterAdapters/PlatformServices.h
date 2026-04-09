#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace Peter::Adapters
{
  struct BootConfig
  {
    std::filesystem::path repoRoot;
    std::filesystem::path userRoot;
    bool developmentMode = true;
  };

  enum class RuntimeMode
  {
    Headless,
    Playable
  };

  struct RuntimeDescriptor
  {
    RuntimeMode mode = RuntimeMode::Headless;
    std::string backendId = "null_headless";
    bool playableRuntimeEnabled = false;
  };

  struct InputState
  {
    bool moveForward = false;
    bool moveBackward = false;
    bool moveLeft = false;
    bool moveRight = false;
    bool sprint = false;
    bool jump = false;
    bool crouch = false;
    bool interact = false;
  };

  struct ActionBinding
  {
    std::string actionId;
    std::string primaryInput;
    std::string secondaryInput;
    std::string displayLabelId;
    std::string categoryId;
    bool remappable = true;
    bool supportsHoldToggle = false;
  };

  struct CameraRigState
  {
    std::string mode = "third_person_ots";
    double shoulderOffsetMeters = 0.75;
    double followDistanceMeters = 4.5;
    double pitchDegrees = 18.0;
  };

  struct PresentationSettings
  {
    bool subtitlesEnabled = true;
    int subtitleScalePercent = 100;
    int textScalePercent = 100;
    bool subtitleBackgroundEnabled = true;
    bool highContrastEnabled = false;
    bool iconRedundancyEnabled = true;
    bool motionComfortEnabled = false;
  };

  struct SceneLoadRequest
  {
    std::string logicalSceneId;
    std::string levelName;
    std::string levelAssetPath;
    std::string spawnPointId;
  };

  struct SceneLoadResult
  {
    bool success = false;
    std::string backendName;
    std::string statusCode = "ok";
    std::string message;
  };

  struct O3DEBootstrapConfig
  {
    std::filesystem::path repoRoot;
    std::filesystem::path userRoot;
    std::filesystem::path engineRoot;
    std::filesystem::path projectRoot;
    bool developmentMode = true;
  };

  struct O3DEBootstrapResult
  {
    bool success = false;
    bool engineRegistered = false;
    bool projectRegistered = false;
    std::string statusCode = "uninitialized";
    std::string message;
    std::filesystem::path engineRoot;
    std::filesystem::path projectRoot;
    std::filesystem::path launcherPath;
    std::filesystem::path assetProcessorPath;
    std::filesystem::path logPath;
    std::filesystem::path runtimeRegistryPath;
  };

  class IInputAdapter
  {
  public:
    virtual ~IInputAdapter() = default;
    virtual std::string ActiveScheme() const = 0;
    virtual InputState SampleInput() const = 0;
    virtual std::vector<ActionBinding> DefaultBindings() const = 0;
  };

  class ICameraAdapter
  {
  public:
    virtual ~ICameraAdapter() = default;
    virtual CameraRigState CurrentRig() const = 0;
    virtual void ApplyRig(const CameraRigState& state) = 0;
  };

  class ISaveAdapter
  {
  public:
    virtual ~ISaveAdapter() = default;
    virtual std::filesystem::path ResolveProfileRoot() const = 0;
    virtual void EnsureDirectory(const std::filesystem::path& directory) const = 0;
  };

  class INavigationAdapter
  {
  public:
    virtual ~INavigationAdapter() = default;
    virtual std::string BackendName() const = 0;
    virtual std::vector<std::string> ResolvePath(
      std::string_view fromNodeId,
      std::string_view toNodeId) const = 0;
  };

  class IAudioAdapter
  {
  public:
    virtual ~IAudioAdapter() = default;
    virtual void PostUiCue(std::string_view cueId) = 0;
    virtual void PostWorldCue(std::string_view cueId) = 0;
    virtual void PostFeedbackCue(std::string_view cueFamily, std::string_view variantId) = 0;
    virtual void PostPrioritizedFeedbackCue(
      std::string_view cueFamily,
      std::string_view variantId,
      int priority,
      bool critical) = 0;
  };

  class IUiAdapter
  {
  public:
    virtual ~IUiAdapter() = default;
    virtual void PresentState(std::string_view stateId) = 0;
    virtual void PresentPrompt(std::string_view prompt) = 0;
    virtual void PresentPanel(std::string_view panelId, std::string_view body) = 0;
    virtual void PresentCreatorPanel(std::string_view panelId, std::string_view body) = 0;
    virtual void PresentTextEditor(std::string_view panelId, std::string_view body) = 0;
    virtual void PresentReplayTimeline(std::string_view panelId, std::string_view body) = 0;
    virtual void PresentMentorSummaryPrompt(std::string_view exportPath, std::string_view body) = 0;
    virtual void ApplyPresentationSettings(const PresentationSettings& settings) = 0;
    virtual void PresentDebugMarkers(const std::vector<std::string>& markerIds) = 0;
    virtual void PresentCompanionFeedback(
      std::string_view calloutToken,
      std::string_view gestureToken) = 0;
  };

  class ISceneAdapter
  {
  public:
    virtual ~ISceneAdapter() = default;
    virtual std::string BackendName() const = 0;
    virtual SceneLoadResult LoadScene(const SceneLoadRequest& request) = 0;
  };

  struct PlatformServices
  {
    std::unique_ptr<IInputAdapter> input;
    std::unique_ptr<ICameraAdapter> camera;
    std::unique_ptr<ISaveAdapter> save;
    std::unique_ptr<INavigationAdapter> navigation;
    std::unique_ptr<IAudioAdapter> audio;
    std::unique_ptr<IUiAdapter> ui;
    std::unique_ptr<ISceneAdapter> scene;
  };

  struct PlatformFactoryResult
  {
    PlatformServices services;
    RuntimeDescriptor descriptor;
    bool available = true;
    std::string statusCode = "ok";
    std::string message;
  };

  [[nodiscard]] std::string ToString(RuntimeMode mode);
  [[nodiscard]] bool TryParseRuntimeMode(std::string_view value, RuntimeMode& mode);
  [[nodiscard]] std::filesystem::path ResolveDefaultO3DERoot();
  [[nodiscard]] O3DEBootstrapResult BootstrapO3DEProject(const O3DEBootstrapConfig& config);
  [[nodiscard]] RuntimeDescriptor BuildRuntimeDescriptor(
    RuntimeMode mode,
    bool playableRuntimeEnabled);
  [[nodiscard]] PlatformFactoryResult CreatePlatformServices(
    const BootConfig& bootConfig,
    const RuntimeDescriptor& runtimeDescriptor);
  PlatformServices CreateNullPlatformServices(const BootConfig& bootConfig);
} // namespace Peter::Adapters
