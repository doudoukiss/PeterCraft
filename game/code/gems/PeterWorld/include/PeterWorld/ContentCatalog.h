#pragma once

#include "PeterWorld/SliceContent.h"

namespace Peter::World::Detail
{
  struct Phase5Catalog
  {
    std::vector<RoomKitDefinition> roomKits;
    std::vector<RoomVariantDefinition> roomVariants;
    std::vector<EncounterPatternDefinition> encounterPatterns;
    std::vector<FeedbackTagDefinition> feedbackTags;
    std::vector<WorldStyleProfileDefinition> styleProfiles;
    std::vector<MissionBlueprintDefinition> missionBlueprints;
    std::vector<PlayableSceneBindingDefinition> sceneBindings;
    std::vector<WorldAnchorDefinition> worldAnchors;
    std::vector<InteractionDefinition> interactions;
    std::vector<PlayableRoomMetricsDefinition> playableRoomMetrics;
    ShippableContentManifest manifest;
  };

  [[nodiscard]] std::filesystem::path ResolveContentRootImpl();
  [[nodiscard]] const Phase5Catalog& GetPhase5Catalog();
  [[nodiscard]] MissionTemplateDefinition BuildMissionTemplateFromBlueprint(
    const MissionBlueprintDefinition& blueprint);
  [[nodiscard]] RaidZoneDefinition BuildRaidZoneFromBlueprint(
    const MissionBlueprintDefinition& blueprint);
} // namespace Peter::World::Detail
