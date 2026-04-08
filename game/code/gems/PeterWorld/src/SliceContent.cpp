#include "PeterWorld/SliceContent.h"

#include "PeterWorld/ContentCatalog.h"
#include "PeterWorkshop/CreatorWorkshop.h"

#include <stdexcept>

namespace Peter::World
{
  HomeBaseDefinition BuildPhase1HomeBase()
  {
    return HomeBaseDefinition{
      "scene.home.workshop_yard",
      "Workshop Yard",
      {
        {"station.home.stash", "Stash Terminal", "Review loot, recovery options, and saved gear.", "ui.station.stash"},
        {"station.home.workbench", "Workbench", "Unlock new upgrades, repair gear, and reclaim favorites.", "ui.station.workbench"},
        {"station.home.companion", "Companion Terminal", "Inspect companion behavior and capability upgrades.", "ui.station.companion"},
        {"station.home.mission_board", "Mission Board", "Choose the next mission template and launch.", "ui.station.mission_board"}
      }};
  }

  RaidZoneDefinition BuildPhase1RaidZone()
  {
    return BuildRaidZoneForMission("mission.salvage_run.machine_silo");
  }

  RaidZoneDefinition BuildRaidZoneForMission(const std::string_view missionId)
  {
    const auto* blueprint = FindMissionBlueprint(missionId);
    if (blueprint == nullptr)
    {
      throw std::runtime_error("Missing mission blueprint.");
    }
    return Detail::BuildRaidZoneFromBlueprint(*blueprint);
  }

  std::filesystem::path ResolveContentRoot()
  {
    return Detail::ResolveContentRootImpl();
  }

  const std::vector<RoomKitDefinition>& BuildPhase5RoomKits()
  {
    return Detail::GetPhase5Catalog().roomKits;
  }

  const RoomKitDefinition* FindRoomKit(const std::string_view roomKitId)
  {
    for (const auto& roomKit : BuildPhase5RoomKits())
    {
      if (roomKit.id == roomKitId)
      {
        return &roomKit;
      }
    }
    return nullptr;
  }

  const std::vector<RoomVariantDefinition>& BuildPhase5RoomVariants()
  {
    return Detail::GetPhase5Catalog().roomVariants;
  }

  const RoomVariantDefinition* FindRoomVariant(const std::string_view roomVariantId)
  {
    for (const auto& roomVariant : BuildPhase5RoomVariants())
    {
      if (roomVariant.id == roomVariantId)
      {
        return &roomVariant;
      }
    }
    return nullptr;
  }

  const std::vector<EncounterPatternDefinition>& BuildPhase5EncounterPatterns()
  {
    return Detail::GetPhase5Catalog().encounterPatterns;
  }

  const EncounterPatternDefinition* FindEncounterPattern(const std::string_view encounterPatternId)
  {
    for (const auto& encounter : BuildPhase5EncounterPatterns())
    {
      if (encounter.id == encounterPatternId)
      {
        return &encounter;
      }
    }
    return nullptr;
  }

  const std::vector<FeedbackTagDefinition>& BuildPhase5FeedbackTags()
  {
    return Detail::GetPhase5Catalog().feedbackTags;
  }

  const FeedbackTagDefinition* FindFeedbackTag(const std::string_view feedbackTagId)
  {
    for (const auto& feedbackTag : BuildPhase5FeedbackTags())
    {
      if (feedbackTag.id == feedbackTagId)
      {
        return &feedbackTag;
      }
    }
    return nullptr;
  }

  const std::vector<WorldStyleProfileDefinition>& BuildPhase5StyleProfiles()
  {
    return Detail::GetPhase5Catalog().styleProfiles;
  }

  const WorldStyleProfileDefinition* FindWorldStyleProfile(const std::string_view styleProfileId)
  {
    for (const auto& profile : BuildPhase5StyleProfiles())
    {
      if (profile.id == styleProfileId)
      {
        return &profile;
      }
    }
    return nullptr;
  }

  const std::vector<MissionBlueprintDefinition>& BuildPhase5MissionBlueprints()
  {
    return Detail::GetPhase5Catalog().missionBlueprints;
  }

  const MissionBlueprintDefinition* FindMissionBlueprint(const std::string_view missionBlueprintId)
  {
    for (const auto& mission : BuildPhase5MissionBlueprints())
    {
      if (mission.id == missionBlueprintId)
      {
        return &mission;
      }
    }
    return nullptr;
  }

  const ShippableContentManifest& BuildPhase5ShippableContentManifest()
  {
    return Detail::GetPhase5Catalog().manifest;
  }

  RoomMetricsSummary BuildRoomMetricsSummary(const std::string_view roomVariantId)
  {
    RoomMetricsSummary summary;
    summary.roomVariantId = std::string(roomVariantId);
    if (const auto* roomVariant = FindRoomVariant(roomVariantId))
    {
      summary.exitCount = roomVariant->exitRoomIds.size();
      summary.extractionPoint = roomVariant->extractionPoint;
      summary.highRiskReward = roomVariant->highRiskReward;
      if (const auto* roomKit = FindRoomKit(roomVariant->kitId))
      {
        summary.connectorClass = roomKit->connectorClass;
        summary.widthMeters = roomKit->widthMeters;
        summary.depthMeters = roomKit->depthMeters;
        summary.heightMeters = roomKit->heightMeters;
      }
    }
    return summary;
  }

  const std::vector<MissionTemplateDefinition>& BuildPhase2MissionTemplates()
  {
    static const std::vector<MissionTemplateDefinition> templates = []() {
      std::vector<MissionTemplateDefinition> built;
      for (const auto& blueprint : BuildPhase5MissionBlueprints())
      {
        built.push_back(Detail::BuildMissionTemplateFromBlueprint(blueprint));
      }
      return built;
    }();

    return templates;
  }

  const MissionTemplateDefinition* FindMissionTemplate(const std::string_view missionId)
  {
    for (const auto& mission : BuildPhase2MissionTemplates())
    {
      if (mission.id == missionId)
      {
        return &mission;
      }
    }
    return nullptr;
  }

  const std::vector<TutorialLessonDefinition>& BuildPhase2TutorialLessons()
  {
    static const std::vector<TutorialLessonDefinition> lessons = {
      TutorialLessonDefinition{
        "lesson.phase2.first_combat",
        "First Combat",
        "Teach the player what a combat action means and why the companion helped.",
        true,
        {
          {"lesson.phase2.first_combat.prompt", "show_prompt", "Stay calm and watch the combat cues.", "gameplay.raid.start", "", "", 0},
          {"lesson.phase2.first_combat.highlight_enemy", "highlight_target", "The patrol unit is the current threat.", "gameplay.combat.action", "", "enemy.machine_patrol.chaser_01", 0},
          {"lesson.phase2.first_combat.wait", "wait_for_event", "Wait for the first combat action to resolve.", "gameplay.combat.action", "gameplay.combat.action", "", 1},
          {"lesson.phase2.first_combat.why", "show_why_panel", "Open the why panel to see why the companion acted.", "creator_tools.explain_panel.opened", "creator_tools.explain_panel.opened", "", 0},
          {"lesson.phase2.first_combat.done", "complete_lesson", "Combat lesson complete.", "gameplay.raid.end", "gameplay.raid.end", "", 0}
        }},
      TutorialLessonDefinition{
        "lesson.phase2.first_repair",
        "First Repair",
        "Teach the player how recovery, repairs, and favorite items work.",
        true,
        {
          {"lesson.phase2.first_repair.prompt", "show_prompt", "Broken gear is recoverable. Let's fix it together.", "gameplay.extraction.failure", "", "", 0},
          {"lesson.phase2.first_repair.grant", "grant_temp_item", "Grant a Tutorial Schematic for the repair lesson.", "gameplay.extraction.failure", "", "item.quest.tutorial_schematic", 0},
          {"lesson.phase2.first_repair.open_panel", "open_panel", "Open the recovery panel in the workshop.", "creator_tools.station.opened", "creator_tools.station.opened", "station.home.workbench", 1},
          {"lesson.phase2.first_repair.wait", "wait_for_event", "Repair or reclaim one item to continue.", "gameplay.workshop.recovery_used", "gameplay.workshop.recovery_used", "", 1},
          {"lesson.phase2.first_repair.done", "complete_lesson", "Repair lesson complete.", "gameplay.workshop.recovery_used", "gameplay.workshop.recovery_used", "", 0}
        }},
      TutorialLessonDefinition{
        "lesson.phase2.mission_choice",
        "Mission Choice",
        "Teach the player how mission templates change the run.",
        true,
        {
          {"lesson.phase2.mission_choice.prompt", "show_prompt", "Different mission templates ask for different choices.", "creator_tools.station.opened", "", "station.home.mission_board", 0},
          {"lesson.phase2.mission_choice.launch", "launch_lesson_mission", "Launch a lesson mission from the board.", "creator_tools.station.opened", "gameplay.mission.selected", "mission.salvage_run.machine_silo", 0},
          {"lesson.phase2.mission_choice.open_panel", "open_panel", "Open the mission comparison panel.", "gameplay.mission.selected", "", "ui.mission_choice", 0},
          {"lesson.phase2.mission_choice.wait", "wait_for_event", "Choose a mission template to continue.", "gameplay.mission.selected", "gameplay.mission.selected", "", 1},
          {"lesson.phase2.mission_choice.done", "complete_lesson", "Mission choice lesson complete.", "gameplay.mission.selected", "gameplay.mission.selected", "", 0}
        }},
      TutorialLessonDefinition{
        "lesson.phase4.change_value",
        "Change A Value",
        "Teach the player how Tinker Mode changes one safe number.",
        true,
        {
          {"lesson.phase4.change_value.open", "open_creator_panel", "Open the creator panel from the workshop terminal.", "creator_tools.station.opened", "", "station.home.workbench", 0},
          {"lesson.phase4.change_value.apply", "apply_tinker_preset", "Apply a close-guard preset and preview the difference.", "creator_tools.station.opened", "creator_tools.tinker.applied", "preset.tinker.companion_close_guard", 0},
          {"lesson.phase4.change_value.replay", "show_replay", "Open the replay to see what your change caused.", "creator_tools.replay.opened", "creator_tools.replay.opened", "scenario.ai.follow_corridor.v1", 0},
          {"lesson.phase4.change_value.done", "complete_lesson", "Value lesson complete.", "creator_tools.tinker.applied", "creator_tools.tinker.applied", "", 0}
        }},
      TutorialLessonDefinition{
        "lesson.phase4.change_rule",
        "Change A Rule",
        "Teach the player how ordered logic cards change companion choices.",
        true,
        {
          {"lesson.phase4.change_rule.open", "open_creator_panel", "Open Logic Mode from the creator panel.", "creator_tools.station.opened", "", "station.home.companion", 0},
          {"lesson.phase4.change_rule.apply", "apply_logic_template", "Apply a starter rule template.", "creator_tools.station.opened", "creator_tools.logic.applied", "logic.template.protect_player", 0},
          {"lesson.phase4.change_rule.why", "show_why_panel", "Read why the companion changed its choice.", "creator_tools.explain_panel.opened", "creator_tools.explain_panel.opened", "", 0},
          {"lesson.phase4.change_rule.done", "complete_lesson", "Rule lesson complete.", "creator_tools.logic.applied", "creator_tools.logic.applied", "", 0}
        }},
      TutorialLessonDefinition{
        "lesson.phase4.read_explanation",
        "Read The Explanation",
        "Teach the player how to compare before and after behavior safely.",
        true,
        {
          {"lesson.phase4.read_explanation.replay", "show_replay", "Open the replay comparison view.", "creator_tools.replay.opened", "creator_tools.replay.opened", "scenario.ai.loot_vs_safety.v1", 0},
          {"lesson.phase4.read_explanation.panel", "open_panel", "Open the explanation panel and read the top reasons.", "creator_tools.explain_panel.opened", "creator_tools.explain_panel.opened", "ui.creator.explain", 0},
          {"lesson.phase4.read_explanation.done", "complete_lesson", "Explanation lesson complete.", "creator_tools.replay.opened", "creator_tools.replay.opened", "", 0}
        }},
      TutorialLessonDefinition{
        "lesson.phase4.first_script",
        "Write Your First Tiny Script",
        "Teach the player how to validate and run a tiny script safely.",
        true,
        {
          {"lesson.phase4.first_script.open", "open_creator_panel", "Open Code Mode from the creator panel.", "creator_tools.station.opened", "", "station.home.workbench", 0},
          {"lesson.phase4.first_script.edit", "edit_script", "Write or load a tiny script template.", "creator_tools.station.opened", "creator_tools.script.validated", "script.template.priority_hint", 0},
          {"lesson.phase4.first_script.validate", "run_script_validation", "Validate the script before activation.", "creator_tools.script.validated", "creator_tools.script.validated", "script.template.priority_hint", 0},
          {"lesson.phase4.first_script.done", "complete_lesson", "Script lesson complete.", "creator_tools.script.validated", "creator_tools.script.validated", "", 0}
        }},
      TutorialLessonDefinition{
        "lesson.phase4.build_mini_mission",
        "Build A Mini Mission",
        "Teach the player how to assemble and launch a local mini mission.",
        true,
        {
          {"lesson.phase4.build_mini_mission.open", "open_creator_panel", "Open Build Mode from the workshop terminal.", "creator_tools.station.opened", "", "station.home.workbench", 0},
          {"lesson.phase4.build_mini_mission.launch", "launch_creator_mission", "Launch a local mini mission from your saved draft.", "creator_tools.mini_mission.launched", "creator_tools.mini_mission.launched", "mini_mission.creator.machine_silo_intro", 0},
          {"lesson.phase4.build_mini_mission.reset", "reset_creator_content", "Reset creator content to compare with the baseline again.", "creator_tools.reset", "creator_tools.reset", "", 0},
          {"lesson.phase4.build_mini_mission.done", "complete_lesson", "Mini mission lesson complete.", "creator_tools.mini_mission.launched", "creator_tools.mini_mission.launched", "", 0}
        }}
    };

    return lessons;
  }

  const TutorialLessonDefinition* FindTutorialLesson(const std::string_view lessonId)
  {
    for (const auto& lesson : BuildPhase2TutorialLessons())
    {
      if (lesson.id == lessonId)
      {
        return &lesson;
      }
    }
    return nullptr;
  }

  const std::vector<MiniMissionRoomBundleDefinition>& BuildPhase4MiniMissionRoomBundles()
  {
    static const std::vector<MiniMissionRoomBundleDefinition> bundles = {
      {"bundle.machine_silo.entry_lane", "Entry Lane", "room.raid.entry_platform", "room.raid.extraction_pad", {"room.raid.entry_platform", "room.raid.patrol_hall", "room.raid.guard_post", "room.raid.extraction_pad"}},
      {"bundle.machine_silo.vault_loop", "Vault Loop", "room.raid.entry_platform", "room.raid.extraction_pad", {"room.raid.entry_platform", "room.raid.guard_post", "room.raid.high_risk_vault", "room.raid.extraction_pad"}}
    };
    return bundles;
  }

  const MiniMissionRoomBundleDefinition* FindMiniMissionRoomBundle(const std::string_view bundleId)
  {
    for (const auto& bundle : BuildPhase4MiniMissionRoomBundles())
    {
      if (bundle.id == bundleId)
      {
        return &bundle;
      }
    }
    return nullptr;
  }

  const std::vector<MiniMissionEnemyGroupDefinition>& BuildPhase4MiniMissionEnemyGroups()
  {
    static const std::vector<MiniMissionEnemyGroupDefinition> groups = {
      {"enemy_group.machine_silo.patrol_pair", "Patrol Pair", {Peter::AI::BuildEnemyUnit("enemy.machine_patrol.chaser_creator", Peter::AI::EnemyVariant::MeleeChaser, "room.raid.patrol_hall"), Peter::AI::BuildEnemyUnit("enemy.machine_patrol.support_creator", Peter::AI::EnemyVariant::AlarmSupport, "room.raid.guard_post")}},
      {"enemy_group.machine_silo.ambush_watch", "Ambush Watch", {Peter::AI::BuildEnemyUnit("enemy.machine_patrol.support_watch", Peter::AI::EnemyVariant::AlarmSupport, "room.raid.guard_post")}}
    };
    return groups;
  }

  const MiniMissionEnemyGroupDefinition* FindMiniMissionEnemyGroup(const std::string_view enemyGroupId)
  {
    for (const auto& group : BuildPhase4MiniMissionEnemyGroups())
    {
      if (group.id == enemyGroupId)
      {
        return &group;
      }
    }
    return nullptr;
  }

  const std::vector<MiniMissionRewardDefinition>& BuildPhase4MiniMissionRewards()
  {
    static const std::vector<MiniMissionRewardDefinition> rewards = {
      {"reward.creator.scrap_bundle", "Scrap Bundle", RewardBundleDefinition{{{"item.salvage.scrap_metal", 2}}, "track.creator_unlocks", "tip.creator.keep_changes_local"}},
      {"reward.creator.nanofiber_bundle", "Nanofiber Bundle", RewardBundleDefinition{{{"item.material.nanofiber", 1}}, "track.creator_unlocks", "tip.creator.logic_cards_are_safe"}}
    };
    return rewards;
  }

  const MiniMissionRewardDefinition* FindMiniMissionReward(const std::string_view rewardId)
  {
    for (const auto& reward : BuildPhase4MiniMissionRewards())
    {
      if (reward.id == rewardId)
      {
        return &reward;
      }
    }
    return nullptr;
  }

  MissionTemplateDefinition BuildMissionFromMiniMissionDraft(const Peter::Workshop::MiniMissionDraftDefinition& draft)
  {
    const auto* bundle = FindMiniMissionRoomBundle(draft.roomBundleId);
    const auto* reward = FindMiniMissionReward(draft.rewardBundleId);

    MissionTemplateDefinition mission;
    mission.id = draft.id;
    mission.displayName = draft.displayName;
    mission.templateType = "creator_mini_mission";
    mission.zoneId = "scene.raid.machine_silo";
    mission.roomIds = bundle == nullptr ? std::vector<std::string>{} : bundle->roomIds;
    mission.objectives = {
      {"objective.creator.collect_goal", "collect_item", draft.lootGoalItemId, "Collect the creator-selected goal item.", 1, false},
      {"objective.creator.extract", "extract", draft.extractionPointId, "Extract from the chosen creator pad.", 1, false}
    };
    mission.rewardBundle = reward == nullptr ? RewardBundleDefinition{} : reward->rewardBundle;
    mission.failRuleId = "fail_rule.standard_extraction";
    mission.recommendedMinutes = 5;
    mission.extractionCountdownSeconds = 6;
    return mission;
  }
} // namespace Peter::World
