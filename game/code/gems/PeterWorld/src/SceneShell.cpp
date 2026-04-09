#include "PeterWorld/SceneShell.h"

#include "PeterAdapters/PlatformServices.h"

namespace Peter::World
{
  SceneShell::SceneShell(
    Peter::Core::EventBus& eventBus,
    Peter::Adapters::ISceneAdapter* sceneAdapter)
    : m_eventBus(eventBus)
    , m_sceneAdapter(sceneAdapter)
  {
  }

  SceneState SceneShell::LoadScene(const std::string& sceneId) const
  {
    const SceneState state{sceneId, "mission.foundation.none", SceneKind::Menu};
    RequestEngineSceneLoad(state);
    return state;
  }

  SceneState SceneShell::LoadHomeBase(const HomeBaseDefinition& homeBase) const
  {
    const SceneState state{homeBase.sceneId, "mission.none.home", SceneKind::HomeBase};
    RequestEngineSceneLoad(state);
    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::Gameplay,
      "gameplay.home_base.loaded",
      {
        {"scene_id", homeBase.sceneId},
        {"station_count", std::to_string(homeBase.stations.size())}
      }});
    return state;
  }

  SceneState SceneShell::LoadRaidZone(const RaidZoneDefinition& raidZone) const
  {
    const SceneState state{raidZone.sceneId, raidZone.missionId, SceneKind::RaidZone};
    RequestEngineSceneLoad(state);
    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::Gameplay,
      "gameplay.raid.loaded",
      {
        {"mission_id", raidZone.missionId},
        {"room_count", std::to_string(raidZone.rooms.size())},
        {"scene_id", raidZone.sceneId}
      }});
    return state;
  }

  SceneState SceneShell::LoadRaidResults(const std::string_view missionId, const bool success) const
  {
    const std::string sceneId = success ? "scene.results.success" : "scene.results.failure";
    const SceneState state{sceneId, std::string(missionId), SceneKind::RaidResults};
    RequestEngineSceneLoad(state);
    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::Gameplay,
      "gameplay.raid_results.loaded",
      {
        {"mission_id", std::string(missionId)},
        {"result", success ? "success" : "failure"},
        {"scene_id", sceneId}
      }});
    return state;
  }

  void SceneShell::RequestEngineSceneLoad(const SceneState& state) const
  {
    const auto* binding = FindPhase7PlayableSceneBinding(state.sceneId);
    if (binding == nullptr)
    {
      m_eventBus.Emit(Peter::Core::Event{
        Peter::Core::EventCategory::Gameplay,
        "gameplay.scene.binding_missing",
        {
          {"mission_id", state.missionId},
          {"scene_id", state.sceneId}
        }});
      return;
    }

    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::Gameplay,
      "gameplay.scene.transition.requested",
      {
        {"level_name", binding->levelName},
        {"mission_id", state.missionId},
        {"scene_id", state.sceneId},
        {"spawn_point_id", binding->spawnPointId}
      }});

    if (m_sceneAdapter == nullptr)
    {
      m_eventBus.Emit(Peter::Core::Event{
        Peter::Core::EventCategory::Gameplay,
        "gameplay.scene.transition.deferred",
        {
          {"level_name", binding->levelName},
          {"scene_id", state.sceneId},
          {"status", "headless_or_no_scene_adapter"}
        }});
      return;
    }

    const auto loadResult = m_sceneAdapter->LoadScene(Peter::Adapters::SceneLoadRequest{
      state.sceneId,
      binding->levelName,
      binding->levelAssetPath,
      binding->spawnPointId});

    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::Gameplay,
      loadResult.success ? "gameplay.scene.transition.loaded" : "gameplay.scene.transition.failed",
      {
        {"backend", loadResult.backendName},
        {"level_name", binding->levelName},
        {"message", loadResult.message},
        {"scene_id", state.sceneId},
        {"status", loadResult.statusCode}
      }});
  }
} // namespace Peter::World
