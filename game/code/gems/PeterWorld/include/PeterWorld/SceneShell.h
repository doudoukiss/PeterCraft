#pragma once

#include "PeterWorld/SliceContent.h"

#include "PeterCore/EventBus.h"

#include <string>

namespace Peter::Adapters
{
  class ISceneAdapter;
}

namespace Peter::World
{
  enum class SceneKind
  {
    Menu,
    HomeBase,
    RaidZone,
    RaidResults
  };

  struct SceneState
  {
    std::string sceneId;
    std::string missionId;
    SceneKind kind = SceneKind::Menu;
  };

  class SceneShell
  {
  public:
    explicit SceneShell(
      Peter::Core::EventBus& eventBus,
      Peter::Adapters::ISceneAdapter* sceneAdapter = nullptr);
    [[nodiscard]] SceneState LoadScene(const std::string& sceneId) const;
    [[nodiscard]] SceneState LoadHomeBase(const HomeBaseDefinition& homeBase) const;
    [[nodiscard]] SceneState LoadRaidZone(const RaidZoneDefinition& raidZone) const;
    [[nodiscard]] SceneState LoadRaidResults(std::string_view missionId, bool success) const;

  private:
    void RequestEngineSceneLoad(const SceneState& state) const;

    Peter::Core::EventBus& m_eventBus;
    Peter::Adapters::ISceneAdapter* m_sceneAdapter = nullptr;
  };
} // namespace Peter::World
