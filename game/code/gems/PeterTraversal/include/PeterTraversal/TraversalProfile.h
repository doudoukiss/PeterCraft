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

  struct TraversalState
  {
    double positionXMeters = 0.0;
    double positionYMeters = 0.0;
    double positionZMeters = 0.0;
    double velocityXMetersPerSecond = 0.0;
    double velocityYMetersPerSecond = 0.0;
    double velocityZMetersPerSecond = 0.0;
    bool grounded = true;
    bool sprinting = false;
    bool crouched = false;
    bool jumpBufferConsumed = false;
    bool jumpRequested = false;
    int jumpBufferMillisecondsRemaining = 0;
    int ledgeForgivenessMillisecondsRemaining = 0;
    std::string cameraMode = "third_person_ots";
    std::string collisionState = "stable";
    double inputToMotionLatencyMs = 0.0;
  };

  [[nodiscard]] TraversalProfile BuildPhase1TraversalProfile();
  [[nodiscard]] TraversalProfile BuildTraversalProfile(const Peter::Core::QualityProfileBase& qualityProfile);
  [[nodiscard]] TraversalProfile BuildTraversalProfile(const Peter::Core::Phase6QualityProfile& qualityProfile);
  [[nodiscard]] TraversalState StepTraversal(
    const TraversalProfile& profile,
    const TraversalState& previousState,
    const Peter::Adapters::InputState& input,
    double deltaTimeSeconds);
  [[nodiscard]] double EvaluateMovementResponsivenessScore(const TraversalProfile& profile);
  [[nodiscard]] std::string DescribeTraversalProfile(const TraversalProfile& profile);
  [[nodiscard]] std::string DescribeTraversalState(const TraversalState& state);
  [[nodiscard]] std::string DescribeCameraFeel(const Peter::Core::CameraFeelProfile& profile);
} // namespace Peter::Traversal
