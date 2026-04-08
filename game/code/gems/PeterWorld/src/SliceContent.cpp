#include "PeterWorld/SliceContent.h"

namespace Peter::World
{
  HomeBaseDefinition BuildPhase1HomeBase()
  {
    return HomeBaseDefinition{
      "scene.home.workshop_yard",
      "Workshop Yard",
      {
        {"station.home.stash", "Stash Terminal", "Deposit and review recovered salvage.", "ui.station.stash"},
        {"station.home.workbench", "Workbench", "Craft one small upgrade from extracted loot.", "ui.station.workbench"},
        {"station.home.companion", "Companion Terminal", "Inspect and tune your helper.", "ui.station.companion"},
        {"station.home.mission_board", "Mission Board", "Launch the next raid.", "ui.station.mission_board"}
      }};
  }

  RaidZoneDefinition BuildPhase1RaidZone()
  {
    return RaidZoneDefinition{
      "scene.raid.machine_silo",
      "mission.vertical_slice.machine_silo",
      "Machine Silo",
      "room.raid.entry_platform",
      "room.raid.extraction_pad",
      {
        {"room.raid.entry_platform", "Entry Platform", "entry", "Blue floodlight", {"room.raid.patrol_hall"}},
        {"room.raid.patrol_hall", "Patrol Hall", "corridor", "Pipe arch", {"room.raid.salvage_nook", "room.raid.guard_post"}},
        {"room.raid.salvage_nook", "Salvage Nook", "optional_loot", "Orange crate stack", {"room.raid.guard_post"}, true},
        {"room.raid.guard_post", "Guard Post", "combat", "Broken console", {"room.raid.high_risk_vault", "room.raid.coolant_walk"}},
        {"room.raid.high_risk_vault", "High Risk Vault", "reward", "Red hazard door", {"room.raid.coolant_walk"}, true, true},
        {"room.raid.coolant_walk", "Coolant Walk", "connector", "Green coolant pipes", {"room.raid.extraction_pad"}},
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
          {Peter::AI::BuildEnemyUnit("enemy.machine_patrol.support_02", Peter::AI::EnemyVariant::AlarmSupport, "room.raid.high_risk_vault")},
          true}
      },
      {"extraction.raid.machine_silo", 5, "world.extraction.begin", "world.extraction.fail"}};
  }
} // namespace Peter::World
