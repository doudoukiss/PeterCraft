#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <string_view>

namespace Peter::Adapters
{
  struct BootConfig
  {
    std::filesystem::path repoRoot;
    std::filesystem::path userRoot;
    bool developmentMode = true;
  };

  class IInputAdapter
  {
  public:
    virtual ~IInputAdapter() = default;
    virtual std::string ActiveScheme() const = 0;
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
  };

  class IAudioAdapter
  {
  public:
    virtual ~IAudioAdapter() = default;
    virtual void PostUiCue(std::string_view cueId) = 0;
  };

  class IUiAdapter
  {
  public:
    virtual ~IUiAdapter() = default;
    virtual void PresentState(std::string_view stateId) = 0;
  };

  struct PlatformServices
  {
    std::unique_ptr<IInputAdapter> input;
    std::unique_ptr<ISaveAdapter> save;
    std::unique_ptr<INavigationAdapter> navigation;
    std::unique_ptr<IAudioAdapter> audio;
    std::unique_ptr<IUiAdapter> ui;
  };

  PlatformServices CreateNullPlatformServices(const BootConfig& bootConfig);
} // namespace Peter::Adapters
