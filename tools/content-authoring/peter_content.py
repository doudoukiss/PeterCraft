from __future__ import annotations

import argparse
import json
import pathlib
import subprocess
import sys


REPO_ROOT = pathlib.Path(__file__).resolve().parents[2]
CONTENT_ROOT = REPO_ROOT / "game" / "data" / "content"
GENERATED_ROOT = REPO_ROOT / "Saved" / "Generated"


def load_json(path: pathlib.Path) -> dict[str, str]:
  with path.open("r", encoding="utf-8-sig") as handle:
    payload = json.load(handle)
  return {str(key): str(value) for key, value in payload.items()}


def write_json(path: pathlib.Path, payload: dict[str, str]) -> None:
  path.parent.mkdir(parents=True, exist_ok=True)
  with path.open("w", encoding="utf-8") as handle:
    json.dump(payload, handle, ensure_ascii=True)
    handle.write("\n")


def load_catalog(kind_dir: str) -> list[dict[str, str]]:
  root = CONTENT_ROOT / kind_dir
  return [load_json(path) for path in sorted(root.glob("*.json"))]


def split_csv(value: str) -> list[str]:
  return [part.strip() for part in value.split(",") if part.strip()]


def scaffold_room(content_id: str, room_id: str, kit_id: str, style_profile_id: str) -> pathlib.Path:
  payload = {
    "id": content_id,
    "room_id": room_id,
    "display_name": content_id.split(".")[-1].replace("_", " ").title(),
    "kit_id": kit_id,
    "style_profile_id": style_profile_id,
    "room_type": "corridor",
    "landmark_label": "Author landmark",
    "exit_room_ids": "",
    "tutorial_anchor_ids": "",
    "companion_hint_anchor_ids": "",
    "feedback_tag_ids": "tag.feedback.danger",
    "optional_path": "false",
    "high_risk_reward": "false",
    "extraction_point": "false",
    "review_record_id": f"review.room.{content_id}",
  }
  path = CONTENT_ROOT / "room-variants" / f"{content_id}.json"
  write_json(path, payload)
  return path


def scaffold_encounter(content_id: str, room_id: str) -> pathlib.Path:
  payload = {
    "id": content_id,
    "display_name": content_id.split(".")[-1].replace("_", " ").title(),
    "room_id": room_id,
    "enemy_units": f"enemy.{content_id}.starter~MeleeChaser~{room_id}",
    "loot_item_ids": "item.salvage.scrap_metal",
    "hazard_ids": "hazard.placeholder",
    "landmark_ids": "landmark.placeholder",
    "companion_hint_ids": "hint.companion.placeholder",
    "tutorial_hook_ids": "hook.tutorial.placeholder",
    "feedback_tag_ids": "tag.feedback.danger",
    "pressure_beat_id": "beat.placeholder",
    "optional": "false",
    "review_record_id": f"review.encounter.{content_id}",
  }
  path = CONTENT_ROOT / "encounter-patterns" / f"{content_id}.json"
  write_json(path, payload)
  return path


def scaffold_mission(content_id: str, scene_id: str, room_variant_ids: list[str], encounter_pattern_ids: list[str]) -> pathlib.Path:
  extraction_target = "room.raid.extraction_pad"
  if room_variant_ids:
    variant_path = CONTENT_ROOT / "room-variants" / f"{room_variant_ids[-1]}.json"
    if variant_path.exists():
      extraction_target = load_json(variant_path).get("room_id", extraction_target)
  payload = {
    "id": content_id,
    "display_name": content_id.split(".")[-1].replace("_", " ").title(),
    "template_family_id": "salvage_run",
    "zone_id": "zone.authored",
    "scene_id": scene_id,
    "zone_display_name": scene_id.split(".")[-1].replace("_", " ").title(),
    "room_variant_ids": ",".join(room_variant_ids),
    "encounter_pattern_ids": ",".join(encounter_pattern_ids),
    "objectives": f"objective.author.collect~collect_item~item.salvage.scrap_metal~Collect one Scrap Metal.~1~false;objective.author.extract~extract~{extraction_target}~Extract safely.~1~false",
    "side_objectives": "",
    "reward_items": "item.salvage.scrap_metal=1",
    "upgrade_track_id": "track.inventory_capacity",
    "lesson_tip_id": "tip.author.preview",
    "fail_rule_id": "fail_rule.standard_extraction",
    "recommended_minutes": "6",
    "extraction_countdown_seconds": "6",
    "feedback_tag_ids": "tag.feedback.danger,tag.feedback.extraction",
    "tutorial_hook_ids": "hook.tutorial.authored",
    "review_record_id": f"review.mission.{content_id}",
  }
  path = CONTENT_ROOT / "mission-blueprints" / f"{content_id}.json"
  write_json(path, payload)
  return path


def preview_mission_graph(content_id: str) -> str:
  mission = load_json(CONTENT_ROOT / "mission-blueprints" / f"{content_id}.json")
  lines = [f"# Mission Preview: {mission['display_name']}", f"- template_family: {mission['template_family_id']}", "- rooms:"]
  for room_variant_id in split_csv(mission["room_variant_ids"]):
    lines.append(f"- {room_variant_id}")
  lines.append("- encounters:")
  for encounter_id in split_csv(mission["encounter_pattern_ids"]):
    lines.append(f"- {encounter_id}")
  lines.append("- feedback_tags:")
  for tag_id in split_csv(mission["feedback_tag_ids"]):
    lines.append(f"- {tag_id}")
  return "\n".join(lines)


def preview_encounter(content_id: str) -> str:
  encounter = load_json(CONTENT_ROOT / "encounter-patterns" / f"{content_id}.json")
  lines = [f"# Encounter Preview: {encounter['display_name']}", f"- room: {encounter['room_id']}", "- enemies:"]
  for enemy in split_csv(encounter["enemy_units"].replace(";", ",")):
    lines.append(f"- {enemy}")
  lines.append("- feedback_tags:")
  for tag_id in split_csv(encounter["feedback_tag_ids"]):
    lines.append(f"- {tag_id}")
  return "\n".join(lines)


def capture_previews() -> list[pathlib.Path]:
  output_dir = GENERATED_ROOT / "content-previews"
  output_dir.mkdir(parents=True, exist_ok=True)
  written: list[pathlib.Path] = []
  for mission in load_catalog("mission-blueprints"):
    path = output_dir / f"{mission['id']}.md"
    path.write_text(preview_mission_graph(mission["id"]) + "\n", encoding="utf-8")
    written.append(path)
  for encounter in load_catalog("encounter-patterns"):
    path = output_dir / f"{encounter['id']}.md"
    path.write_text(preview_encounter(encounter["id"]) + "\n", encoding="utf-8")
    written.append(path)
  return written


def export_room_metrics() -> pathlib.Path:
  output_dir = GENERATED_ROOT / "room-metrics"
  output_dir.mkdir(parents=True, exist_ok=True)
  path = output_dir / "phase5_room_metrics.md"
  kit_index = {item["id"]: item for item in load_catalog("room-kits")}
  lines = ["# Room Metrics Export", ""]
  for room_variant in load_catalog("room-variants"):
    room_kit = kit_index[room_variant["kit_id"]]
    lines.extend([
      f"## {room_variant['id']}",
      f"- connector_class: {room_kit['connector_class']}",
      f"- metrics: {room_kit['width_meters']}x{room_kit['depth_meters']}x{room_kit['height_meters']}",
      f"- exits: {room_variant['exit_room_ids'] or 'none'}",
      f"- extraction_point: {room_variant['extraction_point']}",
      "",
    ])
  path.write_text("\n".join(lines), encoding="utf-8")
  return path


def diff_content() -> pathlib.Path:
  output_dir = GENERATED_ROOT / "content-diffs"
  output_dir.mkdir(parents=True, exist_ok=True)
  path = output_dir / "phase5_content_diff.md"
  result = subprocess.run(
    ["git", "diff", "--", "game/data/content", "docs/quality/content-reviews"],
    cwd=REPO_ROOT,
    capture_output=True,
    text=True,
    check=False,
  )
  body = result.stdout.strip() or "No content diff."
  path.write_text("# Content Diff\n\n```\n" + body + "\n```\n", encoding="utf-8")
  return path


def main() -> int:
  parser = argparse.ArgumentParser(description="PeterCraft Phase 5 content authoring helpers.")
  subparsers = parser.add_subparsers(dest="command", required=True)

  room_parser = subparsers.add_parser("scaffold-room")
  room_parser.add_argument("--id", required=True)
  room_parser.add_argument("--room-id", required=True)
  room_parser.add_argument("--kit-id", required=True)
  room_parser.add_argument("--style-profile-id", required=True)

  encounter_parser = subparsers.add_parser("scaffold-encounter")
  encounter_parser.add_argument("--id", required=True)
  encounter_parser.add_argument("--room-id", required=True)

  mission_parser = subparsers.add_parser("scaffold-mission")
  mission_parser.add_argument("--id", required=True)
  mission_parser.add_argument("--scene-id", required=True)
  mission_parser.add_argument("--room-variant-ids", required=True)
  mission_parser.add_argument("--encounter-pattern-ids", default="")

  preview_mission_parser = subparsers.add_parser("preview-mission-graph")
  preview_mission_parser.add_argument("--id", required=True)

  preview_encounter_parser = subparsers.add_parser("preview-encounter")
  preview_encounter_parser.add_argument("--id", required=True)

  subparsers.add_parser("capture-previews")
  subparsers.add_parser("export-room-metrics")
  subparsers.add_parser("diff-content")

  args = parser.parse_args()

  if args.command == "scaffold-room":
    print(scaffold_room(args.id, args.room_id, args.kit_id, args.style_profile_id))
  elif args.command == "scaffold-encounter":
    print(scaffold_encounter(args.id, args.room_id))
  elif args.command == "scaffold-mission":
    print(
      scaffold_mission(
        args.id,
        args.scene_id,
        split_csv(args.room_variant_ids),
        split_csv(args.encounter_pattern_ids),
      )
    )
  elif args.command == "preview-mission-graph":
    print(preview_mission_graph(args.id))
  elif args.command == "preview-encounter":
    print(preview_encounter(args.id))
  elif args.command == "capture-previews":
    for path in capture_previews():
      print(path)
  elif args.command == "export-room-metrics":
    print(export_room_metrics())
  elif args.command == "diff-content":
    print(diff_content())

  return 0


if __name__ == "__main__":
  sys.exit(main())
