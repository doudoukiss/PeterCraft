#pragma once

#include "PeterCore/EventBus.h"

#include <string>

namespace Peter::World
{
  struct SceneState
  {
    std::string sceneId;
    std::string missionId;
  };

  class SceneShell
  {
  public:
    explicit SceneShell(Peter::Core::EventBus& eventBus);
    [[nodiscard]] SceneState LoadScene(const std::string& sceneId) const;

  private:
    Peter::Core::EventBus& m_eventBus;
  };
} // namespace Peter::World
