#include "PeterWorld/WorldQueryService.h"

#include <algorithm>
#include <cmath>
#include <limits>

namespace Peter::World
{
  namespace
  {
    double DistanceMeters(const WorldPose& pose, const WorldAnchorDefinition& anchor)
    {
      const double dx = pose.xMeters - anchor.xMeters;
      const double dy = pose.yMeters - anchor.yMeters;
      const double dz = pose.zMeters - anchor.zMeters;
      return std::sqrt((dx * dx) + (dy * dy) + (dz * dz));
    }

    const WorldAnchorDefinition* FindAnchorForInteraction(const InteractionDefinition& interaction)
    {
      return FindWorldAnchor(interaction.anchorId);
    }
  } // namespace

  const WorldAnchorDefinition* FindSceneSpawnAnchor(const std::string_view sceneId)
  {
    for (const auto& anchor : BuildPhase7WorldAnchors())
    {
      if (anchor.sceneId == sceneId && anchor.anchorKind == "spawn")
      {
        return &anchor;
      }
    }
    return nullptr;
  }

  const PlayableRoomMetricsDefinition* FindPlayableRoomMetricsForScene(const std::string_view sceneId)
  {
    for (const auto& metrics : BuildPhase7PlayableRoomMetrics())
    {
      if (metrics.sceneId == sceneId)
      {
        return &metrics;
      }
    }
    return nullptr;
  }

  InteractionCandidate ResolveBestInteraction(const std::vector<InteractionCandidate>& candidates)
  {
    InteractionCandidate best;
    bool found = false;
    for (const auto& candidate : candidates)
    {
      if (!candidate.eligible)
      {
        continue;
      }

      if (!found
        || candidate.priority > best.priority
        || (candidate.priority == best.priority && candidate.distanceMeters < best.distanceMeters)
        || (candidate.priority == best.priority
          && std::abs(candidate.distanceMeters - best.distanceMeters) < 0.001
          && candidate.interactionId < best.interactionId))
      {
        best = candidate;
        found = true;
      }
    }
    return best;
  }

  std::string ToString(const ExtractionPhase phase)
  {
    switch (phase)
    {
      case ExtractionPhase::Idle:
        return "idle";
      case ExtractionPhase::Eligible:
        return "eligible";
      case ExtractionPhase::CountdownActive:
        return "countdown_active";
      case ExtractionPhase::Interrupted:
        return "interrupted";
      case ExtractionPhase::Completed:
        return "completed";
      case ExtractionPhase::Failed:
        return "failed";
    }

    return "idle";
  }

  std::string ToString(const SessionPhase phase)
  {
    switch (phase)
    {
      case SessionPhase::Boot:
        return "boot";
      case SessionPhase::HomeBase:
        return "home_base";
      case SessionPhase::MissionSelect:
        return "mission_select";
      case SessionPhase::RaidActive:
        return "raid_active";
      case SessionPhase::Extracting:
        return "extracting";
      case SessionPhase::Results:
        return "results";
      case SessionPhase::Paused:
        return "paused";
      case SessionPhase::ReturningHome:
        return "returning_home";
    }

    return "boot";
  }

  WorldFrameSnapshot CatalogWorldQueryService::CaptureSnapshot(const WorldQueryRequest& request) const
  {
    WorldFrameSnapshot snapshot;
    snapshot.logicalSceneId = request.logicalSceneId;
    snapshot.objectiveId = request.objectiveId;
    snapshot.objectiveLabel = request.objectiveId;

    const WorldAnchorDefinition* roomMarker = nullptr;
    double nearestRoomDistance = std::numeric_limits<double>::max();
    for (const auto& anchor : BuildPhase7WorldAnchors())
    {
      if (anchor.sceneId != request.logicalSceneId || anchor.anchorKind != "room_marker")
      {
        continue;
      }

      const double distance = DistanceMeters(request.playerPose, anchor);
      if (distance < nearestRoomDistance)
      {
        nearestRoomDistance = distance;
        roomMarker = &anchor;
      }
    }

    if (roomMarker != nullptr)
    {
      snapshot.roomId = roomMarker->roomId;
      snapshot.debugMarkerIds = roomMarker->markerIds;
    }

    const auto candidates = QueryInteractions(request);
    const auto best = ResolveBestInteraction(candidates);
    snapshot.activeInteractionId = best.interactionId;
    snapshot.extractionAvailable = best.category == "extraction";
    if (snapshot.debugMarkerIds.empty())
    {
      for (const auto& anchor : BuildPhase7WorldAnchors())
      {
        if (anchor.sceneId == request.logicalSceneId && anchor.anchorKind == "room_marker")
        {
          snapshot.debugMarkerIds.insert(
            snapshot.debugMarkerIds.end(),
            anchor.markerIds.begin(),
            anchor.markerIds.end());
        }
      }
    }
    return snapshot;
  }

  std::vector<InteractionCandidate> CatalogWorldQueryService::QueryInteractions(const WorldQueryRequest& request) const
  {
    std::vector<InteractionCandidate> candidates;
    for (const auto& interaction : BuildPhase7Interactions())
    {
      if (interaction.sceneId != request.logicalSceneId)
      {
        continue;
      }

      const auto* anchor = FindAnchorForInteraction(interaction);
      if (anchor == nullptr)
      {
        continue;
      }

      InteractionCandidate candidate;
      candidate.interactionId = interaction.id;
      candidate.anchorId = interaction.anchorId;
      candidate.roomId = anchor->roomId;
      candidate.displayName = interaction.displayName;
      candidate.category = interaction.category;
      candidate.promptText = interaction.promptText;
      candidate.helpText = interaction.helpText;
      candidate.panelId = interaction.panelId;
      candidate.objectiveId = interaction.objectiveId;
      candidate.priority = interaction.priority;
      candidate.rangeMeters = interaction.rangeMeters;
      candidate.distanceMeters = DistanceMeters(request.playerPose, *anchor);
      candidate.facingScore = 1.0;
      candidate.eligible = candidate.distanceMeters <= std::min(request.interactionRangeMeters, interaction.rangeMeters);
      candidate.denialReason = candidate.eligible ? "" : "Move a little closer and face the object.";
      candidates.push_back(std::move(candidate));
    }
    return candidates;
  }

  InteractionResult CatalogWorldQueryService::ActivateInteraction(
    const std::string_view logicalSceneId,
    const std::string_view interactionId)
  {
    InteractionResult result;
    result.interactionId = std::string(interactionId);
    result.sceneId = std::string(logicalSceneId);

    const auto* interaction = FindInteractionDefinition(interactionId);
    if (interaction == nullptr)
    {
      result.statusCode = "interaction_missing";
      result.message = "The requested interaction is not in the playable catalog.";
      return result;
    }

    result.category = interaction->category;
    result.success = true;
    result.statusCode = "activated";
    result.message = "Activated " + interaction->displayName + ".";
    ++m_activationCounts[result.interactionId];
    return result;
  }
} // namespace Peter::World
