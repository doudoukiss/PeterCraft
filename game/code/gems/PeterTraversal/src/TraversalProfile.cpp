#include "PeterTraversal/TraversalProfile.h"

#include <sstream>

namespace Peter::Traversal
{
  TraversalProfile BuildPhase1TraversalProfile()
  {
    return BuildTraversalProfile(Peter::Core::BuildDefaultPhase6QualityProfile());
  }

  TraversalProfile BuildTraversalProfile(const Peter::Core::Phase6QualityProfile& qualityProfile)
  {
    TraversalProfile profile;
    profile.accelerationMetersPerSecondSquared = qualityProfile.movement.accelerationMetersPerSecondSquared;
    profile.decelerationMetersPerSecondSquared = qualityProfile.movement.decelerationMetersPerSecondSquared;
    profile.jumpBufferMilliseconds = qualityProfile.movement.jumpBufferMilliseconds;
    profile.ledgeForgivenessMilliseconds = qualityProfile.movement.ledgeForgivenessMilliseconds;
    profile.cameraRig.mode = qualityProfile.camera.motionComfortEnabled ? "third_person_ots_comfort" : "third_person_ots";
    profile.cameraRig.followDistanceMeters = qualityProfile.camera.followDistanceMeters;
    profile.cameraRig.pitchDegrees = qualityProfile.camera.pitchDegrees;
    profile.cameraRig.shoulderOffsetMeters = qualityProfile.camera.shoulderOffsetMeters;
    return profile;
  }

  double EvaluateMovementResponsivenessScore(const TraversalProfile& profile)
  {
    return (profile.accelerationMetersPerSecondSquared * 0.45) +
      (profile.decelerationMetersPerSecondSquared * 0.35) +
      (static_cast<double>(profile.ledgeForgivenessMilliseconds) / 40.0) +
      (static_cast<double>(profile.jumpBufferMilliseconds) / 50.0);
  }

  std::string DescribeTraversalProfile(const TraversalProfile& profile)
  {
    std::ostringstream output;
    output << "walk=" << profile.walkSpeedMetersPerSecond << "m/s"
           << ", sprint x" << profile.sprintMultiplier
           << ", jump=" << profile.jumpHeightMeters << "m"
           << ", accel=" << profile.accelerationMetersPerSecondSquared
           << ", decel=" << profile.decelerationMetersPerSecondSquared
           << ", camera=" << profile.cameraRig.mode;
    return output.str();
  }

  std::string DescribeCameraFeel(const Peter::Core::CameraFeelProfile& profile)
  {
    std::ostringstream output;
    output << "smoothing=" << profile.smoothingStrength
           << ", sensitivity_curve=" << profile.sensitivityCurveExponent
           << ", follow_distance=" << profile.followDistanceMeters
           << ", motion_comfort=" << (profile.motionComfortEnabled ? "true" : "false");
    return output.str();
  }
} // namespace Peter::Traversal
