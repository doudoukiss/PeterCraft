#include "PeterTraversal/TraversalProfile.h"

#include <sstream>

namespace Peter::Traversal
{
  TraversalProfile BuildPhase1TraversalProfile()
  {
    TraversalProfile profile;
    profile.cameraRig.mode = "third_person_ots";
    profile.cameraRig.followDistanceMeters = 4.5;
    profile.cameraRig.pitchDegrees = 18.0;
    profile.cameraRig.shoulderOffsetMeters = 0.75;
    return profile;
  }

  std::string DescribeTraversalProfile(const TraversalProfile& profile)
  {
    std::ostringstream output;
    output << "walk=" << profile.walkSpeedMetersPerSecond << "m/s"
           << ", sprint x" << profile.sprintMultiplier
           << ", jump=" << profile.jumpHeightMeters << "m"
           << ", camera=" << profile.cameraRig.mode;
    return output.str();
  }
} // namespace Peter::Traversal
