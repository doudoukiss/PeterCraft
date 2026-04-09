#include "PeterWorld/ContentCatalog.h"

#include <algorithm>
#include <fstream>
#include <iterator>
#include <stdexcept>

namespace Peter::World::Detail
{
  namespace
  {
    std::string Trim(std::string value)
    {
      const auto begin = value.find_first_not_of(" \r\n\t");
      if (begin == std::string::npos)
      {
        return {};
      }

      const auto end = value.find_last_not_of(" \r\n\t");
      return value.substr(begin, end - begin + 1);
    }

    std::vector<std::string> SplitList(const std::string_view serialized, const char delimiter = ',')
    {
      std::vector<std::string> values;
      std::size_t cursor = 0;
      while (cursor < serialized.size())
      {
        const auto separator = serialized.find(delimiter, cursor);
        const auto token = serialized.substr(
          cursor,
          separator == std::string_view::npos ? serialized.size() - cursor : separator - cursor);
        const auto trimmed = Trim(std::string(token));
        if (!trimmed.empty())
        {
          values.push_back(trimmed);
        }

        if (separator == std::string_view::npos)
        {
          break;
        }
        cursor = separator + 1;
      }
      return values;
    }

    bool ParseBool(const std::string_view value)
    {
      return value == "true" || value == "1" || value == "yes";
    }

    Peter::Core::StructuredFields ParseFields(const std::filesystem::path& path)
    {
      Peter::Core::StructuredFields fields;
      if (!std::filesystem::exists(path))
      {
        return fields;
      }

      std::ifstream input(path);
      const std::string content(
        (std::istreambuf_iterator<char>(input)),
        std::istreambuf_iterator<char>());

      std::size_t cursor = 0;
      while (true)
      {
        const auto keyStart = content.find('"', cursor);
        if (keyStart == std::string::npos)
        {
          break;
        }

        const auto keyEnd = content.find('"', keyStart + 1);
        const auto valueStart = content.find('"', keyEnd + 1);
        const auto valueEnd = content.find('"', valueStart + 1);
        if (keyEnd == std::string::npos || valueStart == std::string::npos || valueEnd == std::string::npos)
        {
          break;
        }

        fields[content.substr(keyStart + 1, keyEnd - keyStart - 1)] =
          content.substr(valueStart + 1, valueEnd - valueStart - 1);
        cursor = valueEnd + 1;
      }

      return fields;
    }

    std::vector<std::filesystem::path> SortedJsonFiles(const std::filesystem::path& root)
    {
      std::vector<std::filesystem::path> files;
      if (!std::filesystem::exists(root))
      {
        return files;
      }

      for (const auto& entry : std::filesystem::directory_iterator(root))
      {
        if (entry.is_regular_file() && entry.path().extension() == ".json")
        {
          files.push_back(entry.path());
        }
      }

      std::sort(files.begin(), files.end());
      return files;
    }

    std::filesystem::path ResolveContentRootFrom(std::filesystem::path start)
    {
      if (start.empty())
      {
        return {};
      }

      if (std::filesystem::is_regular_file(start))
      {
        start = start.parent_path();
      }

      auto current = std::filesystem::absolute(start);
      while (!current.empty())
      {
        const auto candidate = current / "game" / "data" / "content";
        if (std::filesystem::exists(candidate))
        {
          return candidate;
        }

        if (current == current.root_path())
        {
          break;
        }
        current = current.parent_path();
      }

      return {};
    }

    Peter::AI::EnemyVariant ParseEnemyVariant(const std::string_view variantId)
    {
      return variantId == "AlarmSupport"
        ? Peter::AI::EnemyVariant::AlarmSupport
        : Peter::AI::EnemyVariant::MeleeChaser;
    }

    Peter::AI::EnemyUnit ParseEnemyUnit(const std::string_view serialized)
    {
      const auto pieces = SplitList(serialized, '~');
      return Peter::AI::BuildEnemyUnit(
        pieces.size() > 0 ? pieces[0] : "enemy.machine_patrol.chaser_01",
        pieces.size() > 1 ? ParseEnemyVariant(pieces[1]) : Peter::AI::EnemyVariant::MeleeChaser,
        pieces.size() > 2 ? pieces[2] : "room.raid.patrol_hall");
    }

    std::vector<Peter::AI::EnemyUnit> ParseEnemyUnits(const std::string_view serialized)
    {
      std::vector<Peter::AI::EnemyUnit> enemies;
      for (const auto& token : SplitList(serialized, ';'))
      {
        enemies.push_back(ParseEnemyUnit(token));
      }
      return enemies;
    }

    MissionObjectiveDefinition ParseObjective(const std::string_view serialized)
    {
      const auto pieces = SplitList(serialized, '~');
      MissionObjectiveDefinition objective;
      if (pieces.size() > 0) objective.id = pieces[0];
      if (pieces.size() > 1) objective.kind = pieces[1];
      if (pieces.size() > 2) objective.targetId = pieces[2];
      if (pieces.size() > 3) objective.description = pieces[3];
      if (pieces.size() > 4) objective.requiredCount = std::stoi(pieces[4]);
      if (pieces.size() > 5) objective.optional = ParseBool(pieces[5]);
      return objective;
    }

    std::vector<MissionObjectiveDefinition> ParseObjectives(const std::string_view serialized)
    {
      std::vector<MissionObjectiveDefinition> objectives;
      for (const auto& token : SplitList(serialized, ';'))
      {
        objectives.push_back(ParseObjective(token));
      }
      return objectives;
    }

    RewardBundleDefinition ParseRewardBundle(const Peter::Core::StructuredFields& fields)
    {
      RewardBundleDefinition bundle;
      if (const auto items = fields.find("reward_items"); items != fields.end())
      {
        bundle.guaranteedItems = Peter::Inventory::ParseLedger(items->second);
      }
      if (const auto track = fields.find("upgrade_track_id"); track != fields.end())
      {
        bundle.upgradeTrackId = track->second;
      }
      if (const auto tip = fields.find("lesson_tip_id"); tip != fields.end())
      {
        bundle.lessonTipId = tip->second;
      }
      return bundle;
    }

    RoomKitDefinition LoadRoomKit(const std::filesystem::path& path)
    {
      const auto fields = ParseFields(path);
      return RoomKitDefinition{
        fields.at("id"),
        fields.at("display_name"),
        fields.at("archetype"),
        fields.at("connector_class"),
        std::stoi(fields.at("width_meters")),
        std::stoi(fields.at("depth_meters")),
        std::stoi(fields.at("height_meters")),
        fields.at("nav_expectation"),
        fields.at("lighting_expectation"),
        fields.at("review_record_id")};
    }

    RoomVariantDefinition LoadRoomVariant(const std::filesystem::path& path)
    {
      const auto fields = ParseFields(path);
      RoomVariantDefinition variant;
      variant.id = fields.at("id");
      variant.roomId = fields.at("room_id");
      variant.displayName = fields.at("display_name");
      variant.kitId = fields.at("kit_id");
      variant.styleProfileId = fields.at("style_profile_id");
      variant.roomType = fields.at("room_type");
      variant.landmarkLabel = fields.at("landmark_label");
      variant.exitRoomIds = SplitList(fields.at("exit_room_ids"));
      variant.tutorialAnchorIds = SplitList(fields.at("tutorial_anchor_ids"));
      variant.companionHintAnchorIds = SplitList(fields.at("companion_hint_anchor_ids"));
      variant.feedbackTagIds = SplitList(fields.at("feedback_tag_ids"));
      variant.optionalPath = ParseBool(fields.at("optional_path"));
      variant.highRiskReward = ParseBool(fields.at("high_risk_reward"));
      variant.extractionPoint = ParseBool(fields.at("extraction_point"));
      variant.reviewRecordId = fields.at("review_record_id");
      return variant;
    }

    EncounterPatternDefinition LoadEncounterPattern(const std::filesystem::path& path)
    {
      const auto fields = ParseFields(path);
      EncounterPatternDefinition pattern;
      pattern.id = fields.at("id");
      pattern.displayName = fields.at("display_name");
      pattern.roomId = fields.at("room_id");
      pattern.enemies = ParseEnemyUnits(fields.at("enemy_units"));
      pattern.lootItemIds = SplitList(fields.at("loot_item_ids"));
      pattern.hazardIds = SplitList(fields.at("hazard_ids"));
      pattern.landmarkIds = SplitList(fields.at("landmark_ids"));
      pattern.companionHintMomentIds = SplitList(fields.at("companion_hint_ids"));
      pattern.tutorialHookIds = SplitList(fields.at("tutorial_hook_ids"));
      pattern.feedbackTagIds = SplitList(fields.at("feedback_tag_ids"));
      pattern.pressureBeatId = fields.at("pressure_beat_id");
      pattern.optional = ParseBool(fields.at("optional"));
      pattern.reviewRecordId = fields.at("review_record_id");
      return pattern;
    }

    FeedbackTagDefinition LoadFeedbackTag(const std::filesystem::path& path)
    {
      const auto fields = ParseFields(path);
      return FeedbackTagDefinition{fields.at("id"), fields.at("display_name"), fields.at("category"),
        fields.at("cue_family"), fields.at("cue_variant"), fields.at("summary")};
    }

    WorldStyleProfileDefinition LoadStyleProfile(const std::filesystem::path& path)
    {
      const auto fields = ParseFields(path);
      return WorldStyleProfileDefinition{fields.at("id"), fields.at("display_name"), fields.at("visual_motif"),
        fields.at("prop_language"), fields.at("color_hierarchy"), fields.at("signage_grammar"),
        fields.at("faction_style")};
    }

    MissionBlueprintDefinition LoadMissionBlueprint(const std::filesystem::path& path)
    {
      const auto fields = ParseFields(path);
      MissionBlueprintDefinition blueprint;
      blueprint.id = fields.at("id");
      blueprint.displayName = fields.at("display_name");
      blueprint.templateFamilyId = fields.at("template_family_id");
      blueprint.zoneId = fields.at("zone_id");
      blueprint.sceneId = fields.at("scene_id");
      blueprint.zoneDisplayName = fields.at("zone_display_name");
      blueprint.roomVariantIds = SplitList(fields.at("room_variant_ids"));
      blueprint.encounterPatternIds = SplitList(fields.at("encounter_pattern_ids"));
      blueprint.objectives = ParseObjectives(fields.at("objectives"));
      blueprint.sideObjectives = ParseObjectives(fields.at("side_objectives"));
      blueprint.rewardBundle = ParseRewardBundle(fields);
      blueprint.failRuleId = fields.at("fail_rule_id");
      blueprint.recommendedMinutes = std::stoi(fields.at("recommended_minutes"));
      blueprint.extractionCountdownSeconds = std::stoi(fields.at("extraction_countdown_seconds"));
      blueprint.feedbackTagIds = SplitList(fields.at("feedback_tag_ids"));
      blueprint.tutorialHookIds = SplitList(fields.at("tutorial_hook_ids"));
      blueprint.reviewRecordId = fields.at("review_record_id");
      return blueprint;
    }

    PlayableSceneBindingDefinition LoadSceneBinding(const std::filesystem::path& path)
    {
      const auto fields = ParseFields(path);
      PlayableSceneBindingDefinition binding;
      binding.sceneId = fields.at("scene_id");
      binding.displayName = fields.at("display_name");
      binding.levelName = fields.at("level_name");
      binding.levelAssetPath = fields.at("level_asset_path");
      binding.spawnPointId = fields.at("spawn_point_id");
      binding.transitionCardId = fields.at("transition_card_id");
      const auto proofRoom = fields.find("proof_room");
      binding.proofRoom = proofRoom != fields.end() && ParseBool(proofRoom->second);
      return binding;
    }

    WorldAnchorDefinition LoadWorldAnchor(const std::filesystem::path& path)
    {
      const auto fields = ParseFields(path);
      WorldAnchorDefinition anchor;
      anchor.id = fields.at("id");
      anchor.sceneId = fields.at("scene_id");
      anchor.displayName = fields.at("display_name");
      anchor.roomId = fields.at("room_id");
      anchor.anchorKind = fields.at("anchor_kind");
      anchor.entityName = fields.at("entity_name");
      anchor.xMeters = std::stod(fields.at("x_meters"));
      anchor.yMeters = std::stod(fields.at("y_meters"));
      anchor.zMeters = std::stod(fields.at("z_meters"));
      anchor.markerIds = SplitList(fields.at("marker_ids"));
      return anchor;
    }

    InteractionDefinition LoadInteractionDefinition(const std::filesystem::path& path)
    {
      const auto fields = ParseFields(path);
      InteractionDefinition definition;
      definition.id = fields.at("id");
      definition.sceneId = fields.at("scene_id");
      definition.anchorId = fields.at("anchor_id");
      definition.displayName = fields.at("display_name");
      definition.category = fields.at("category");
      definition.promptText = fields.at("prompt_text");
      definition.helpText = fields.at("help_text");
      definition.panelId = fields.at("panel_id");
      definition.objectiveId = fields.at("objective_id");
      definition.priority = std::stoi(fields.at("priority"));
      definition.rangeMeters = std::stod(fields.at("range_meters"));
      definition.facingThreshold = std::stod(fields.at("facing_threshold"));
      definition.promptCooldownMilliseconds = std::stoi(fields.at("prompt_cooldown_ms"));
      definition.holdToInteract = ParseBool(fields.at("hold_to_interact"));
      return definition;
    }

    PlayableRoomMetricsDefinition LoadPlayableRoomMetricsDefinition(const std::filesystem::path& path)
    {
      const auto fields = ParseFields(path);
      PlayableRoomMetricsDefinition definition;
      definition.id = fields.at("id");
      definition.sceneId = fields.at("scene_id");
      definition.displayName = fields.at("display_name");
      definition.traversalTimeSeconds = std::stoi(fields.at("traversal_time_seconds"));
      definition.coverDensityLabel = fields.at("cover_density_label");
      definition.landmarkQualityLabel = fields.at("landmark_quality_label");
      definition.companionPathSafetyLabel = fields.at("companion_path_safety_label");
      definition.extractionReadabilityLabel = fields.at("extraction_readability_label");
      definition.reviewRecordId = fields.at("review_record_id");
      return definition;
    }

    ShippableContentManifest LoadManifest(const std::filesystem::path& path)
    {
      const auto fields = ParseFields(path);
      return ShippableContentManifest{
        fields.at("id"),
        SplitList(fields.at("room_variant_ids")),
        SplitList(fields.at("encounter_pattern_ids")),
        SplitList(fields.at("mission_blueprint_ids")),
        SplitList(fields.at("style_profile_ids"))};
    }
  } // namespace

  std::filesystem::path ResolveContentRootImpl()
  {
    if (const auto fromCurrent = ResolveContentRootFrom(std::filesystem::current_path()); !fromCurrent.empty())
    {
      return fromCurrent;
    }

    if (const auto fromSource = ResolveContentRootFrom(std::filesystem::path(__FILE__)); !fromSource.empty())
    {
      return fromSource;
    }

    throw std::runtime_error("Could not resolve PeterCraft shipped content root.");
  }

  const Phase5Catalog& GetPhase5Catalog()
  {
    static const Phase5Catalog catalog = []() {
      Phase5Catalog loaded;
      const auto contentRoot = ResolveContentRootImpl();
      for (const auto& path : SortedJsonFiles(contentRoot / "room-kits")) loaded.roomKits.push_back(LoadRoomKit(path));
      for (const auto& path : SortedJsonFiles(contentRoot / "room-variants")) loaded.roomVariants.push_back(LoadRoomVariant(path));
      for (const auto& path : SortedJsonFiles(contentRoot / "encounter-patterns")) loaded.encounterPatterns.push_back(LoadEncounterPattern(path));
      for (const auto& path : SortedJsonFiles(contentRoot / "feedback-tags")) loaded.feedbackTags.push_back(LoadFeedbackTag(path));
      for (const auto& path : SortedJsonFiles(contentRoot / "style-profiles")) loaded.styleProfiles.push_back(LoadStyleProfile(path));
      for (const auto& path : SortedJsonFiles(contentRoot / "mission-blueprints")) loaded.missionBlueprints.push_back(LoadMissionBlueprint(path));
      for (const auto& path : SortedJsonFiles(contentRoot / "scene-bindings")) loaded.sceneBindings.push_back(LoadSceneBinding(path));
      for (const auto& path : SortedJsonFiles(contentRoot / "world-anchors")) loaded.worldAnchors.push_back(LoadWorldAnchor(path));
      for (const auto& path : SortedJsonFiles(contentRoot / "interactions")) loaded.interactions.push_back(LoadInteractionDefinition(path));
      for (const auto& path : SortedJsonFiles(contentRoot / "playable-room-metrics")) loaded.playableRoomMetrics.push_back(LoadPlayableRoomMetricsDefinition(path));
      const auto manifests = SortedJsonFiles(contentRoot / "content-manifests");
      if (!manifests.empty())
      {
        loaded.manifest = LoadManifest(manifests.front());
      }
      return loaded;
    }();

    return catalog;
  }

  MissionTemplateDefinition BuildMissionTemplateFromBlueprint(const MissionBlueprintDefinition& blueprint)
  {
    MissionTemplateDefinition mission;
    mission.id = blueprint.id;
    mission.displayName = blueprint.displayName;
    mission.templateType = blueprint.templateFamilyId;
    mission.zoneId = blueprint.sceneId;
    mission.objectives = blueprint.objectives;
    mission.sideObjectives = blueprint.sideObjectives;
    mission.rewardBundle = blueprint.rewardBundle;
    mission.failRuleId = blueprint.failRuleId;
    mission.recommendedMinutes = blueprint.recommendedMinutes;
    mission.extractionCountdownSeconds = blueprint.extractionCountdownSeconds;

    for (const auto& variantId : blueprint.roomVariantIds)
    {
      const auto roomIt = std::find_if(
        GetPhase5Catalog().roomVariants.begin(),
        GetPhase5Catalog().roomVariants.end(),
        [&](const auto& variant) { return variant.id == variantId; });
      if (roomIt != GetPhase5Catalog().roomVariants.end())
      {
        mission.roomIds.push_back(roomIt->roomId);
      }
    }

    return mission;
  }

  RaidZoneDefinition BuildRaidZoneFromBlueprint(const MissionBlueprintDefinition& blueprint)
  {
    RaidZoneDefinition raidZone;
    raidZone.sceneId = blueprint.sceneId;
    raidZone.missionId = blueprint.id;
    raidZone.displayName = blueprint.zoneDisplayName;

    for (const auto& variantId : blueprint.roomVariantIds)
    {
      const auto roomIt = std::find_if(
        GetPhase5Catalog().roomVariants.begin(),
        GetPhase5Catalog().roomVariants.end(),
        [&](const auto& variant) { return variant.id == variantId; });
      if (roomIt == GetPhase5Catalog().roomVariants.end())
      {
        continue;
      }

      raidZone.rooms.push_back(RoomDefinition{
        roomIt->roomId,
        roomIt->displayName,
        roomIt->roomType,
        roomIt->landmarkLabel,
        roomIt->exitRoomIds,
        roomIt->optionalPath,
        roomIt->highRiskReward,
        roomIt->extractionPoint});
    }

    for (const auto& encounterId : blueprint.encounterPatternIds)
    {
      const auto encounterIt = std::find_if(
        GetPhase5Catalog().encounterPatterns.begin(),
        GetPhase5Catalog().encounterPatterns.end(),
        [&](const auto& encounter) { return encounter.id == encounterId; });
      if (encounterIt != GetPhase5Catalog().encounterPatterns.end())
      {
        raidZone.encounters.push_back(EncounterDefinition{
          encounterIt->id,
          encounterIt->roomId,
          encounterIt->enemies,
          encounterIt->optional});
      }
    }

    if (!raidZone.rooms.empty())
    {
      raidZone.entryRoomId = raidZone.rooms.front().id;
      raidZone.extractionRoomId = raidZone.rooms.back().id;
      for (const auto& room : raidZone.rooms)
      {
        if (room.extractionPoint)
        {
          raidZone.extractionRoomId = room.id;
          break;
        }
      }
    }

    raidZone.extraction = ExtractionSettings{
      "extraction." + blueprint.id,
      blueprint.extractionCountdownSeconds,
      "world.extraction.begin",
      "world.extraction.fail"};
    return raidZone;
  }
} // namespace Peter::World::Detail
