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

  struct SceneTransitionState
  {
    SceneState previousScene;
    SceneState currentScene;
    std::string spawnPointId;
    std::string transitionCardId;
    std::string statusCode = "idle";
    double lastDurationMs = 0.0;
  };

  class SceneShell
  {
  public:
    explicit SceneShell(
      Peter::Core::EventBus& eventBus,
      Peter::Adapters::ISceneAdapter* sceneAdapter = nullptr);
    [[nodiscard]] SceneState LoadScene(const std::string& sceneId);
    [[nodiscard]] SceneState LoadHomeBase(const HomeBaseDefinition& homeBase);
    [[nodiscard]] SceneState LoadRaidZone(const RaidZoneDefinition& raidZone);
    [[nodiscard]] SceneState LoadRaidResults(std::string_view missionId, bool success);
    [[nodiscard]] const SceneTransitionState& LastTransition() const;

  private:
    void RequestEngineSceneLoad(const SceneState& state);

    Peter::Core::EventBus& m_eventBus;
    Peter::Adapters::ISceneAdapter* m_sceneAdapter = nullptr;
    SceneTransitionState m_lastTransition;
  };
} // namespace Peter::World
