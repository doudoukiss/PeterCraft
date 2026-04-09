#include "PeterTraversal/TraversalProfile.h"

#include <algorithm>
#include <cmath>
#include <sstream>

namespace Peter::Traversal
{
  TraversalProfile BuildPhase1TraversalProfile()
  {
    return BuildTraversalProfile(Peter::Core::BuildDefaultPhase6QualityProfile());
  }

  TraversalProfile BuildTraversalProfile(const Peter::Core::QualityProfileBase& qualityProfile)
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

  TraversalProfile BuildTraversalProfile(const Peter::Core::Phase6QualityProfile& qualityProfile)
  {
    return BuildTraversalProfile(static_cast<const Peter::Core::QualityProfileBase&>(qualityProfile));
  }

  namespace
  {
    double Magnitude(const double x, const double y)
    {
      return std::sqrt((x * x) + (y * y));
    }

    double MoveTowards(const double current, const double target, const double maxDelta)
    {
      if (std::abs(target - current) <= maxDelta)
      {
        return target;
      }
      return current + (target > current ? maxDelta : -maxDelta);
    }
  } // namespace

  TraversalState StepTraversal(
    const TraversalProfile& profile,
    const TraversalState& previousState,
    const Peter::Adapters::InputState& input,
    const double deltaTimeSeconds)
  {
    TraversalState next = previousState;

    const double clampedDeltaTime = std::max(0.001, deltaTimeSeconds);
    const double inputMagnitude = std::min(1.0, Magnitude(input.moveAxisX, input.moveAxisY));
    const bool wantsToMove = inputMagnitude > 0.05;
    const bool wantsToSprint = input.sprint || input.sprintHeld || input.sprintPressed;
    const bool wantsToJump = input.jump || input.jumpHeld || input.jumpPressed;
    const bool wantsToCrouch = input.crouch || input.crouchHeld || input.crouchPressed;

    next.sprinting = wantsToMove && wantsToSprint;
    next.crouched = wantsToCrouch;
    next.cameraMode = profile.cameraRig.mode;
    next.jumpRequested = wantsToJump;
    next.jumpBufferConsumed = false;

    const double baseSpeed = profile.walkSpeedMetersPerSecond *
      (next.sprinting ? profile.sprintMultiplier : 1.0) *
      (next.crouched ? 0.6 : 1.0);
    const double targetSpeed = baseSpeed * inputMagnitude;
    const double desiredVelocityX = inputMagnitude <= 0.05 ? 0.0 : (input.moveAxisX / std::max(0.001, inputMagnitude)) * targetSpeed;
    const double desiredVelocityY = inputMagnitude <= 0.05 ? 0.0 : (input.moveAxisY / std::max(0.001, inputMagnitude)) * targetSpeed;

    const double planarCurrentSpeed = Magnitude(next.velocityXMetersPerSecond, next.velocityYMetersPerSecond);
    const double accel = (targetSpeed > planarCurrentSpeed)
      ? profile.accelerationMetersPerSecondSquared
      : profile.decelerationMetersPerSecondSquared;
    const double maxVelocityDelta = accel * clampedDeltaTime;
    next.velocityXMetersPerSecond = MoveTowards(next.velocityXMetersPerSecond, desiredVelocityX, maxVelocityDelta);
    next.velocityYMetersPerSecond = MoveTowards(next.velocityYMetersPerSecond, desiredVelocityY, maxVelocityDelta);

    if (wantsToJump)
    {
      next.jumpBufferMillisecondsRemaining = profile.jumpBufferMilliseconds;
    }
    else
    {
      next.jumpBufferMillisecondsRemaining = std::max(
        0,
        next.jumpBufferMillisecondsRemaining - static_cast<int>(clampedDeltaTime * 1000.0));
    }

    if (!next.grounded)
    {
      next.ledgeForgivenessMillisecondsRemaining = std::max(
        0,
        next.ledgeForgivenessMillisecondsRemaining - static_cast<int>(clampedDeltaTime * 1000.0));
    }
    else
    {
      next.ledgeForgivenessMillisecondsRemaining = profile.ledgeForgivenessMilliseconds;
    }

    const bool canJumpNow = next.grounded || next.ledgeForgivenessMillisecondsRemaining > 0;
    if (next.jumpBufferMillisecondsRemaining > 0 && canJumpNow)
    {
      next.velocityZMetersPerSecond = std::sqrt(std::max(0.1, profile.jumpHeightMeters) * 2.0 * 9.81);
      next.grounded = false;
      next.jumpBufferConsumed = true;
      next.jumpBufferMillisecondsRemaining = 0;
    }
    else if (!next.grounded)
    {
      next.velocityZMetersPerSecond -= 9.81 * clampedDeltaTime;
    }

    next.positionXMeters += next.velocityXMetersPerSecond * clampedDeltaTime;
    next.positionYMeters += next.velocityYMetersPerSecond * clampedDeltaTime;
    next.positionZMeters += next.velocityZMetersPerSecond * clampedDeltaTime;
    if (next.positionZMeters <= 0.0)
    {
      next.positionZMeters = 0.0;
      next.velocityZMetersPerSecond = 0.0;
      next.grounded = true;
    }

    const double resultingPlanarSpeed = Magnitude(next.velocityXMetersPerSecond, next.velocityYMetersPerSecond);
    next.collisionState = next.grounded ? "stable" : "airborne";
    if (next.crouched)
    {
      next.collisionState = "crouched";
    }
    next.inputToMotionLatencyMs = wantsToMove
      ? std::clamp(8.0 + ((1.0 - inputMagnitude) * 20.0) + (next.crouched ? 6.0 : 0.0), 8.0, 85.0)
      : 0.0;
    if (wantsToMove && resultingPlanarSpeed < 0.25)
    {
      next.inputToMotionLatencyMs += 4.0;
    }

    return next;
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

  std::string DescribeTraversalState(const TraversalState& state)
  {
    std::ostringstream output;
    output << "speed="
           << std::sqrt(
                (state.velocityXMetersPerSecond * state.velocityXMetersPerSecond) +
                (state.velocityYMetersPerSecond * state.velocityYMetersPerSecond))
           << "m/s"
           << ", grounded=" << (state.grounded ? "true" : "false")
           << ", crouched=" << (state.crouched ? "true" : "false")
           << ", jump_buffer_used=" << (state.jumpBufferConsumed ? "true" : "false")
           << ", camera=" << state.cameraMode
           << ", collision=" << state.collisionState
           << ", latency_ms=" << state.inputToMotionLatencyMs;
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
