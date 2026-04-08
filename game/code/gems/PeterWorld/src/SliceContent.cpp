#include "PeterWorld/SliceContent.h"

#include "PeterWorkshop/CreatorWorkshop.h"

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
    return RaidZoneDefinition{
      "scene.raid.machine_silo",
      "mission.salvage_run.machine_silo",
      "Machine Silo",
      "room.raid.entry_platform",
      "room.raid.extraction_pad",
      {
        {"room.raid.entry_platform", "Entry Platform", "entry", "Blue floodlight", {"room.raid.patrol_hall"}},
        {"room.raid.patrol_hall", "Patrol Hall", "corridor", "Pipe arch", {"room.raid.salvage_nook", "room.raid.guard_post"}},
        {"room.raid.salvage_nook", "Salvage Nook", "optional_loot", "Orange crate stack", {"room.raid.guard_post"}, true},
        {"room.raid.guard_post", "Guard Post", "combat", "Broken console", {"room.raid.high_risk_vault", "room.raid.coolant_walk"}},
        {"room.raid.high_risk_vault", "High Risk Vault", "reward", "Red hazard door", {"room.raid.coolant_walk"}, true, true},
        {"room.raid.coolant_walk", "Coolant Walk", "connector", "Green coolant pipes", {"room.raid.generator_room", "room.raid.extraction_pad"}},
        {"room.raid.generator_room", "Generator Room", "objective", "White machine core", {"room.raid.extraction_pad"}, true},
        {"room.raid.extraction_pad", "Extraction Pad", "extraction", "Amber beacon", {}, false, false, true}
      },
      {
        {"encounter.raid.patrol_hall", "room.raid.patrol_hall",
          {Peter::AI::BuildEnemyUnit("enemy.machine_patrol.chaser_01", Peter::AI::EnemyVariant::MeleeChaser, "room.raid.patrol_hall")}},
        {"encounter.raid.guard_post", "room.raid.guard_post",
          {
            Peter::AI::BuildEnemyUnit("enemy.machine_patrol.chaser_02", Peter::AI::EnemyVariant::MeleeChaser, "room.raid.guard_post"),
            Peter::AI::BuildEnemyUnit("enemy.machine_patrol.support_01", Peter::AI::EnemyVariant::AlarmSupport, "room.raid.guard_post")
          }},
        {"encounter.raid.high_risk_vault", "room.raid.high_risk_vault",
          {
            Peter::AI::BuildEnemyUnit("enemy.machine_patrol.chaser_03", Peter::AI::EnemyVariant::MeleeChaser, "room.raid.high_risk_vault"),
            Peter::AI::BuildEnemyUnit("enemy.machine_patrol.support_02", Peter::AI::EnemyVariant::AlarmSupport, "room.raid.high_risk_vault")
          },
          true}
      },
      {"extraction.raid.machine_silo", 6, "world.extraction.begin", "world.extraction.fail"}};
  }

  const std::vector<MissionTemplateDefinition>& BuildPhase2MissionTemplates()
  {
    static const std::vector<MissionTemplateDefinition> templates = {
      MissionTemplateDefinition{
        "mission.salvage_run.machine_silo",
        "Salvage Run",
        "salvage_run",
        "scene.raid.machine_silo",
        {
          "room.raid.entry_platform",
          "room.raid.patrol_hall",
          "room.raid.salvage_nook",
          "room.raid.guard_post",
          "room.raid.high_risk_vault",
          "room.raid.coolant_walk",
          "room.raid.extraction_pad"
        },
        {
          {"objective.salvage.collect_scrap", "collect_item", "item.salvage.scrap_metal", "Collect three Scrap Metal.", 3, false},
          {"objective.salvage.extract", "extract", "room.raid.extraction_pad", "Reach the extraction pad and leave safely.", 1, false}
        },
        {
          {"objective.salvage.power_cell", "collect_item", "item.salvage.power_cell", "Recover a Power Cell from the high-risk vault.", 1, true}
        },
        {{{"item.salvage.scrap_metal", 2}}, "track.inventory_capacity", "tip.raid.keep_rarity_feedback"},
        "fail_rule.standard_extraction",
        9,
        6},
      MissionTemplateDefinition{
        "mission.recover_artifact.machine_silo",
        "Recover Artifact",
        "recover_artifact",
        "scene.raid.machine_silo",
        {
          "room.raid.entry_platform",
          "room.raid.guard_post",
          "room.raid.high_risk_vault",
          "room.raid.extraction_pad"
        },
        {
          {"objective.artifact.recover", "collect_item", "item.quest.artifact_seed", "Recover the Artifact Seed.", 1, false},
          {"objective.artifact.extract", "extract", "room.raid.extraction_pad", "Extract with the Artifact Seed.", 1, false}
        },
        {
          {"objective.artifact.side_salvage", "collect_item", "item.material.nanofiber", "Grab bonus Nanofiber on the way out.", 1, true}
        },
        {{{"item.material.nanofiber", 1}}, "track.player_tools", "tip.raid.quest_items_are_safe"},
        "fail_rule.artifact_preserved",
        10,
        6},
      MissionTemplateDefinition{
        "mission.activate_machine.machine_silo",
        "Activate Machine",
        "activate_machine",
        "scene.raid.machine_silo",
        {
          "room.raid.entry_platform",
          "room.raid.guard_post",
          "room.raid.generator_room",
          "room.raid.extraction_pad"
        },
        {
          {"objective.machine.activate", "activate_target", "room.raid.generator_room", "Activate the machine core in the generator room.", 1, false},
          {"objective.machine.extract", "extract", "room.raid.extraction_pad", "Extract after the machine core comes online.", 1, false}
        },
        {
          {"objective.machine.repair", "repair_target", "item.tool.field_wrench", "Use the Field Wrench to repair a damaged panel.", 1, true}
        },
        {{{"item.material.nanofiber", 1}}, "track.player_tools", "tip.raid.tools_support_objectives"},
        "fail_rule.machine_timeout",
        8,
        6},
      MissionTemplateDefinition{
        "mission.escort_companion.machine_silo",
        "Escort Companion",
        "escort_companion",
        "scene.raid.machine_silo",
        {
          "room.raid.entry_platform",
          "room.raid.patrol_hall",
          "room.raid.guard_post",
          "room.raid.coolant_walk",
          "room.raid.extraction_pad"
        },
        {
          {"objective.escort.keep_companion_safe", "escort", "companion", "Escort the companion through the silo.", 1, false},
          {"objective.escort.extract", "extract", "room.raid.extraction_pad", "Extract together.", 1, false}
        },
        {
          {"objective.escort.scan_loot", "scan_loot", "room.raid.salvage_nook", "Let the companion ping optional loot.", 1, true}
        },
        {{{"item.salvage.scrap_metal", 1}, {"item.material.nanofiber", 1}}, "track.companion_capabilities", "tip.raid.watch_companion_highlights"},
        "fail_rule.companion_downed",
        11,
        6},
      MissionTemplateDefinition{
        "mission.timed_extraction.machine_silo",
        "Timed Extraction",
        "timed_extraction",
        "scene.raid.machine_silo",
        {
          "room.raid.entry_platform",
          "room.raid.guard_post",
          "room.raid.high_risk_vault",
          "room.raid.extraction_pad"
        },
        {
          {"objective.timed.loot", "collect_item", "item.salvage.power_cell", "Grab the Power Cell before the timer expires.", 1, false},
          {"objective.timed.extract", "extract", "room.raid.extraction_pad", "Reach extraction before the countdown ends.", 1, false}
        },
        {
          {"objective.timed.side_console", "activate_target", "room.raid.generator_room", "Activate the side console for bonus salvage.", 1, true}
        },
        {{{"item.salvage.power_cell", 1}}, "track.creator_unlocks", "tip.raid_reduced_time_pressure_available"},
        "fail_rule.timer_expired",
        7,
        5}
    };

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
