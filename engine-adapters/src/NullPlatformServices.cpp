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
    };

    class NullAudioAdapter final : public IAudioAdapter
    {
    public:
      void PostUiCue(std::string_view cueId) override
      {
        std::cout << "[audio] " << cueId << '\n';
      }
    };

    class NullUiAdapter final : public IUiAdapter
    {
    public:
      void PresentState(std::string_view stateId) override
      {
        std::cout << "[ui] " << stateId << '\n';
      }
    };
  } // namespace

  PlatformServices CreateNullPlatformServices(const BootConfig& bootConfig)
  {
    PlatformServices services;
    services.input = std::make_unique<NullInputAdapter>();
    services.save = std::make_unique<NullSaveAdapter>(bootConfig.userRoot);
    services.navigation = std::make_unique<NullNavigationAdapter>();
    services.audio = std::make_unique<NullAudioAdapter>();
    services.ui = std::make_unique<NullUiAdapter>();
    return services;
  }
} // namespace Peter::Adapters
