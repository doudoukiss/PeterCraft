#pragma once

#include "PeterAdapters/PlatformServices.h"
#include "PeterCore/QualityProfile.h"

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
    double accelerationMetersPerSecondSquared = 18.0;
    double decelerationMetersPerSecondSquared = 22.0;
    int ledgeForgivenessMilliseconds = 120;
    int jumpBufferMilliseconds = 100;
    Peter::Adapters::CameraRigState cameraRig;
    bool controllerSupported = true;
    bool mouseKeyboardSupported = true;
  };

  [[nodiscard]] TraversalProfile BuildPhase1TraversalProfile();
  [[nodiscard]] TraversalProfile BuildTraversalProfile(const Peter::Core::Phase6QualityProfile& qualityProfile);
  [[nodiscard]] double EvaluateMovementResponsivenessScore(const TraversalProfile& profile);
  [[nodiscard]] std::string DescribeTraversalProfile(const TraversalProfile& profile);
  [[nodiscard]] std::string DescribeCameraFeel(const Peter::Core::CameraFeelProfile& profile);
} // namespace Peter::Traversal
