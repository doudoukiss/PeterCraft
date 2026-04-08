#include "PeterWorld/SceneShell.h"

namespace Peter::World
{
  SceneShell::SceneShell(Peter::Core::EventBus& eventBus)
    : m_eventBus(eventBus)
  {
  }

  SceneState SceneShell::LoadScene(const std::string& sceneId) const
  {
    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::Gameplay,
      "gameplay.scene.loaded",
      {
        {"mission_id", "mission.foundation.none"},
        {"scene_id", sceneId}
      }});

    return SceneState{sceneId, "mission.foundation.none", SceneKind::Menu};
  }

  SceneState SceneShell::LoadHomeBase(const HomeBaseDefinition& homeBase) const
  {
    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::Gameplay,
      "gameplay.home_base.loaded",
      {
        {"scene_id", homeBase.sceneId},
        {"station_count", std::to_string(homeBase.stations.size())}
      }});
    return SceneState{homeBase.sceneId, "mission.none.home", SceneKind::HomeBase};
  }

  SceneState SceneShell::LoadRaidZone(const RaidZoneDefinition& raidZone) const
  {
    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::Gameplay,
      "gameplay.raid.loaded",
      {
        {"mission_id", raidZone.missionId},
        {"room_count", std::to_string(raidZone.rooms.size())},
        {"scene_id", raidZone.sceneId}
      }});
    return SceneState{raidZone.sceneId, raidZone.missionId, SceneKind::RaidZone};
  }

  SceneState SceneShell::LoadRaidResults(const std::string_view missionId, const bool success) const
  {
    const std::string sceneId = success ? "scene.results.success" : "scene.results.failure";
    m_eventBus.Emit(Peter::Core::Event{
      Peter::Core::EventCategory::Gameplay,
      "gameplay.raid_results.loaded",
      {
        {"mission_id", std::string(missionId)},
        {"result", success ? "success" : "failure"},
        {"scene_id", sceneId}
      }});
    return SceneState{sceneId, std::string(missionId), SceneKind::RaidResults};
  }
} // namespace Peter::World
