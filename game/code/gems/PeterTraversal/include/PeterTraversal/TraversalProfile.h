#pragma once

#include "PeterAdapters/PlatformServices.h"

#include <string>

namespace Peter::Traversal
{
  struct TraversalProfile
  {
    double walkSpeedMetersPerSecond = 4.2;
    double sprintMultiplier = 1.45;
    double jumpHeightMeters = 1.1;
    double crouchHeightScale = 0.65;
    double interactionRangeMeters = 2.5;
    Peter::Adapters::CameraRigState cameraRig;
    bool controllerSupported = true;
    bool mouseKeyboardSupported = true;
  };

  [[nodiscard]] TraversalProfile BuildPhase1TraversalProfile();
  [[nodiscard]] std::string DescribeTraversalProfile(const TraversalProfile& profile);
} // namespace Peter::Traversal
