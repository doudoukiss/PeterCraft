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

    return SceneState{sceneId, "mission.foundation.none"};
  }
} // namespace Peter::World
