#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace Peter::Core
{
  struct TargetHardwareProfile
  {
    std::string id;
    std::string operatingSystem;
    std::string cpuClass;
    int memoryGigabytes = 16;
    std::string storageClass;
    std::string gpuClass;
  };

  struct QualityBudgetProfile
  {
    int fpsTarget = 60;
    double frameTimeP95Ms = 20.0;
    double coldBootBudgetMs = 5000.0;
    double transitionBudgetMs = 2000.0;
    int peakWorkingSetBudgetMb = 2048;
    double aiDecisionP95Ms = 2.0;
    double uiPanelRenderP95Ms = 1.0;
    double singleSaveBudgetMs = 50.0;
    double fullLoadBudgetMs = 250.0;
    double fullSaveBudgetMs = 400.0;
  };

  struct MovementFeelProfile
  {
    double accelerationMetersPerSecondSquared = 18.0;
    double decelerationMetersPerSecondSquared = 22.0;
    double jumpReadabilityScale = 1.0;
    int jumpBufferMilliseconds = 100;
    int ledgeForgivenessMilliseconds = 120;
    double companionSpacingMeters = 4.0;
  };

  struct CameraFeelProfile
  {
    double smoothingStrength = 0.55;
    double sensitivityCurveExponent = 1.1;
    double shoulderOffsetMeters = 0.75;
    double followDistanceMeters = 4.5;
    double pitchDegrees = 18.0;
    bool motionComfortEnabled = true;
  };

  struct CombatReadabilityProfile
  {
    int telegraphWindowMilliseconds = 650;
    int hitStopMilliseconds = 55;
    int lowHealthThreshold = 30;
    double lineOfFireWidthMeters = 1.25;
    std::string statusClarityMode = "layered_icons";
  };

  struct FeedbackProfile
  {
    int maxConcurrentWorldCues = 8;
    int maxCriticalCuesPerBeat = 2;
    std::vector<std::string> requiredCueFamilies;
    std::map<std::string, int, std::less<>> cuePriorityByFamily;
  };

  struct Phase6QualityProfile
  {
    std::string id;
    std::string displayName;
    TargetHardwareProfile targetHardware;
    QualityBudgetProfile budgets;
    MovementFeelProfile movement;
    CameraFeelProfile camera;
    CombatReadabilityProfile combat;
    FeedbackProfile feedback;
    bool subtitleBackgroundRequired = true;
    bool highContrastRequired = true;
    bool iconRedundancyRequired = true;
    bool motionComfortRequired = true;
  };

  [[nodiscard]] std::filesystem::path ResolveQualityProfileRoot();
  [[nodiscard]] Phase6QualityProfile BuildDefaultPhase6QualityProfile();
  [[nodiscard]] Phase6QualityProfile LoadPhase6QualityProfile();
  [[nodiscard]] std::string DescribeTargetHardwareProfile(const TargetHardwareProfile& profile);
} // namespace Peter::Core
