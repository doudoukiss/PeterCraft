#pragma once

#include "PeterWorld/SliceContent.h"

#include <map>
#include <string>
#include <string_view>
#include <vector>

namespace Peter::World
{
  struct WorldPose
  {
    double xMeters = 0.0;
    double yMeters = 0.0;
    double zMeters = 0.0;
    double velocityXMetersPerSecond = 0.0;
    double velocityYMetersPerSecond = 0.0;
    double velocityZMetersPerSecond = 0.0;
  };

  struct WorldQueryRequest
  {
    std::string logicalSceneId;
    WorldPose playerPose;
    double interactionRangeMeters = 2.5;
    std::string objectiveId;
  };

  struct InteractionCandidate
  {
    std::string interactionId;
    std::string anchorId;
    std::string roomId;
    std::string displayName;
    std::string category;
    std::string promptText;
    std::string helpText;
    std::string panelId;
    std::string objectiveId;
    int priority = 0;
    double rangeMeters = 2.5;
    double distanceMeters = 0.0;
    double facingScore = 1.0;
    bool eligible = false;
    std::string denialReason;
  };

  struct InteractionResult
  {
    bool success = false;
    std::string interactionId;
    std::string sceneId;
    std::string category;
    std::string statusCode = "uninitialized";
    std::string message;
  };

  struct WorldFrameSnapshot
  {
    std::string logicalSceneId;
    std::string roomId;
    std::string objectiveId;
    std::string objectiveLabel;
    std::string activeInteractionId;
    bool extractionAvailable = false;
    std::vector<std::string> debugMarkerIds;
  };

  enum class ExtractionPhase
  {
    Idle,
    Eligible,
    CountdownActive,
    Interrupted,
    Completed,
    Failed
  };

  struct ExtractionRuntimeState
  {
    std::string interactionId;
    std::string roomId;
    ExtractionPhase phase = ExtractionPhase::Idle;
    int countdownTotalSeconds = 0;
    int countdownRemainingSeconds = 0;
    bool reducedTimePressure = false;
    std::string failureReason;
  };

  enum class SessionPhase
  {
    Boot,
    HomeBase,
    MissionSelect,
    RaidActive,
    Extracting,
    Results,
    Paused,
    ReturningHome
  };

  struct PlayableSessionState
  {
    SessionPhase phase = SessionPhase::Boot;
    std::string sceneId;
    std::string missionId;
    std::string roomId;
    std::string currentObjectiveId;
    std::string activeInteractionId;
    std::string inputScheme = "mouse_keyboard";
    std::string cameraMode = "third_person_ots";
  };

  class IWorldQueryService
  {
  public:
    virtual ~IWorldQueryService() = default;
    [[nodiscard]] virtual WorldFrameSnapshot CaptureSnapshot(const WorldQueryRequest& request) const = 0;
    [[nodiscard]] virtual std::vector<InteractionCandidate> QueryInteractions(const WorldQueryRequest& request) const = 0;
    virtual InteractionResult ActivateInteraction(std::string_view logicalSceneId, std::string_view interactionId) = 0;
  };

  class CatalogWorldQueryService final : public IWorldQueryService
  {
  public:
    [[nodiscard]] WorldFrameSnapshot CaptureSnapshot(const WorldQueryRequest& request) const override;
    [[nodiscard]] std::vector<InteractionCandidate> QueryInteractions(const WorldQueryRequest& request) const override;
    InteractionResult ActivateInteraction(std::string_view logicalSceneId, std::string_view interactionId) override;

  private:
    mutable std::map<std::string, int, std::less<>> m_activationCounts;
  };

  [[nodiscard]] const WorldAnchorDefinition* FindSceneSpawnAnchor(std::string_view sceneId);
  [[nodiscard]] const PlayableRoomMetricsDefinition* FindPlayableRoomMetricsForScene(std::string_view sceneId);
  [[nodiscard]] InteractionCandidate ResolveBestInteraction(const std::vector<InteractionCandidate>& candidates);
  [[nodiscard]] std::string ToString(ExtractionPhase phase);
  [[nodiscard]] std::string ToString(SessionPhase phase);
} // namespace Peter::World
