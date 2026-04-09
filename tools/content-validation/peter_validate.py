from __future__ import annotations

import argparse
import json
import pathlib
import subprocess
import sys
from collections import defaultdict, deque
from typing import Iterable

from jsonschema import Draft202012Validator


REPO_ROOT = pathlib.Path(__file__).resolve().parents[2]
SCHEMA_ROOT = REPO_ROOT / "game" / "data" / "schemas"
EXAMPLES_ROOT = REPO_ROOT / "game" / "data" / "examples"
INVALID_ROOT = REPO_ROOT / "game" / "data" / "fixtures" / "invalid"
CONTENT_ROOT = REPO_ROOT / "game" / "data" / "content"
REVIEW_ROOT = REPO_ROOT / "docs" / "quality" / "content-reviews"

CONTENT_SCHEMA_BY_DIR = {
  "room-kits": "room_kits.schema.json",
  "room-variants": "room_variants.schema.json",
  "encounter-patterns": "encounter_patterns.schema.json",
  "mission-blueprints": "mission_blueprints.schema.json",
  "feedback-tags": "feedback_tags.schema.json",
  "style-profiles": "style_profiles.schema.json",
  "content-manifests": "content_manifests.schema.json",
  "quality-profiles": "quality_profiles.schema.json",
}


def schema_for_json(path: pathlib.Path) -> pathlib.Path:
  if CONTENT_ROOT in path.parents:
    schema_name = CONTENT_SCHEMA_BY_DIR.get(path.parent.name)
    if schema_name is None:
      raise FileNotFoundError(f"No content schema found for {path.relative_to(REPO_ROOT)}")
    return SCHEMA_ROOT / schema_name

  token = path.name.split(".")[0]
  schema_path = SCHEMA_ROOT / f"{token}.schema.json"
  if not schema_path.exists():
    raise FileNotFoundError(f"No schema found for {path.relative_to(REPO_ROOT)}")
  return schema_path


def load_json(path: pathlib.Path) -> object:
  with path.open("r", encoding="utf-8-sig") as handle:
    return json.load(handle)


def validate_file(path: pathlib.Path) -> list[str]:
  schema = load_json(schema_for_json(path))
  payload = load_json(path)
  validator = Draft202012Validator(schema)
  return [
    f"{path.relative_to(REPO_ROOT)}: {error.message}"
    for error in validator.iter_errors(payload)
  ]


def iter_content_files() -> list[pathlib.Path]:
  return sorted(CONTENT_ROOT.glob("*/*.json"))


def validate_expected_valid(files: Iterable[pathlib.Path]) -> list[str]:
  issues: list[str] = []
  for path in files:
    issues.extend(validate_file(path))
  return issues


def validate_expected_invalid(files: Iterable[pathlib.Path]) -> list[str]:
  issues: list[str] = []
  for path in files:
    if not validate_file(path):
      issues.append(f"{path.relative_to(REPO_ROOT)}: expected validation failure but passed")
  return issues


def collect_changed_files(base_ref: str) -> list[pathlib.Path]:
  command = ["git", "diff", "--name-only", "--diff-filter=ACMRT", f"{base_ref}...HEAD"]
  result = subprocess.run(command, cwd=REPO_ROOT, capture_output=True, text=True, check=True)
  files: list[pathlib.Path] = []
  for line in result.stdout.splitlines():
    path = REPO_ROOT / line.strip()
    if path.suffix == ".json" and path.exists():
      files.append(path)
  return files


def split_csv(value: str) -> list[str]:
  return [part.strip() for part in value.split(",") if part.strip()]


def load_content_registry() -> dict[str, dict[str, dict[str, str]]]:
  registry: dict[str, dict[str, dict[str, str]]] = defaultdict(dict)
  for path in iter_content_files():
    payload = load_json(path)
    if not isinstance(payload, dict) or "id" not in payload:
      continue
    registry[path.parent.name][str(payload["id"])] = {str(key): str(value) for key, value in payload.items()}
  return registry


def review_path(kind_dir: str, content_id: str) -> pathlib.Path:
  if kind_dir == "room-variants":
    return REVIEW_ROOT / "rooms" / f"{content_id}.md"
  if kind_dir == "encounter-patterns":
    return REVIEW_ROOT / "encounters" / f"{content_id}.md"
  return REVIEW_ROOT / "missions" / f"{content_id}.md"


def validate_content_registry() -> list[str]:
  registry = load_content_registry()
  issues: list[str] = []

  room_kits = registry["room-kits"]
  room_variants = registry["room-variants"]
  encounter_patterns = registry["encounter-patterns"]
  mission_blueprints = registry["mission-blueprints"]
  feedback_tags = registry["feedback-tags"]
  style_profiles = registry["style-profiles"]
  manifests = registry["content-manifests"]
  quality_profiles = registry["quality-profiles"]

  room_ids: dict[str, str] = {}
  for content_id, payload in room_variants.items():
    room_id = payload.get("room_id", "")
    if room_id in room_ids:
      issues.append(
        f"game/data/content/room-variants/{content_id}.json: duplicate room_id {room_id} also used by {room_ids[room_id]}"
      )
    room_ids[room_id] = content_id

    if payload.get("kit_id") not in room_kits:
      issues.append(f"game/data/content/room-variants/{content_id}.json: unknown kit_id")
    if payload.get("style_profile_id") not in style_profiles:
      issues.append(f"game/data/content/room-variants/{content_id}.json: unknown style_profile_id")
    for exit_room_id in split_csv(payload.get("exit_room_ids", "")):
      if exit_room_id not in room_ids and exit_room_id not in {p.get('room_id', '') for p in room_variants.values()}:
        issues.append(f"game/data/content/room-variants/{content_id}.json: unknown exit_room_id {exit_room_id}")

    review = review_path("room-variants", content_id)
    if not review.exists():
      issues.append(f"{review.relative_to(REPO_ROOT)}: missing review record for shippable room variant")

  for content_id, payload in encounter_patterns.items():
    if payload.get("room_id") not in room_ids:
      issues.append(f"game/data/content/encounter-patterns/{content_id}.json: encounter room_id is not in the room catalog")
    for tag_id in split_csv(payload.get("feedback_tag_ids", "")):
      if tag_id not in feedback_tags:
        issues.append(f"game/data/content/encounter-patterns/{content_id}.json: unknown feedback tag {tag_id}")
    if review_path("encounter-patterns", content_id).exists() is False:
      issues.append(
        f"{review_path('encounter-patterns', content_id).relative_to(REPO_ROOT)}: missing review record for shippable encounter"
      )

  for content_id, payload in mission_blueprints.items():
    variant_ids = split_csv(payload.get("room_variant_ids", ""))
    encounter_ids = split_csv(payload.get("encounter_pattern_ids", ""))
    if len(variant_ids) < 2:
      issues.append(f"game/data/content/mission-blueprints/{content_id}.json: mission needs at least two room variants")
    if not payload.get("feedback_tag_ids"):
      issues.append(f"game/data/content/mission-blueprints/{content_id}.json: mission is missing feedback tags")

    for variant_id in variant_ids:
      if variant_id not in room_variants:
        issues.append(f"game/data/content/mission-blueprints/{content_id}.json: unknown room variant {variant_id}")
    for encounter_id in encounter_ids:
      if encounter_id not in encounter_patterns:
        issues.append(f"game/data/content/mission-blueprints/{content_id}.json: unknown encounter pattern {encounter_id}")
      elif encounter_patterns[encounter_id].get("room_id") not in {room_variants[v]["room_id"] for v in variant_ids if v in room_variants}:
        issues.append(
          f"game/data/content/mission-blueprints/{content_id}.json: encounter {encounter_id} anchors to a room outside the mission path"
        )
    for tag_id in split_csv(payload.get("feedback_tag_ids", "")):
      if tag_id not in feedback_tags:
        issues.append(f"game/data/content/mission-blueprints/{content_id}.json: unknown feedback tag {tag_id}")

    graph = {room_variants[v]["room_id"]: split_csv(room_variants[v].get("exit_room_ids", "")) for v in variant_ids if v in room_variants}
    if graph:
      start = room_variants[variant_ids[0]]["room_id"]
      extraction_rooms = {
        room_variants[v]["room_id"]
        for v in variant_ids
        if v in room_variants and room_variants[v].get("extraction_point") == "true"
      }
      if not extraction_rooms:
        issues.append(f"game/data/content/mission-blueprints/{content_id}.json: mission path has no extraction room")
      else:
        seen = {start}
        queue: deque[str] = deque([start])
        while queue:
          room_id = queue.popleft()
          for next_room in graph.get(room_id, []):
            if next_room in graph and next_room not in seen:
              seen.add(next_room)
              queue.append(next_room)
        if not extraction_rooms.intersection(seen):
          issues.append(f"game/data/content/mission-blueprints/{content_id}.json: extraction room is unreachable from the first room")

    review = review_path("mission-blueprints", content_id)
    if not review.exists():
      issues.append(f"{review.relative_to(REPO_ROOT)}: missing review record for shippable mission")

  for manifest_id, payload in manifests.items():
    for room_variant_id in split_csv(payload.get("room_variant_ids", "")):
      if room_variant_id not in room_variants:
        issues.append(f"game/data/content/content-manifests/{manifest_id}.json: unknown room variant {room_variant_id}")
    for encounter_id in split_csv(payload.get("encounter_pattern_ids", "")):
      if encounter_id not in encounter_patterns:
        issues.append(f"game/data/content/content-manifests/{manifest_id}.json: unknown encounter pattern {encounter_id}")
    for mission_id in split_csv(payload.get("mission_blueprint_ids", "")):
      if mission_id not in mission_blueprints:
        issues.append(f"game/data/content/content-manifests/{manifest_id}.json: unknown mission blueprint {mission_id}")
    for style_id in split_csv(payload.get("style_profile_ids", "")):
      if style_id not in style_profiles:
        issues.append(f"game/data/content/content-manifests/{manifest_id}.json: unknown style profile {style_id}")

  for profile_id, payload in quality_profiles.items():
    required_families = split_csv(payload.get("feedback_required_families", ""))
    priority_map = {
      token.split(":", 1)[0]: token.split(":", 1)[1]
      for token in split_csv(payload.get("feedback_priorities", ""))
      if ":" in token
    }
    if not required_families:
      issues.append(f"game/data/content/quality-profiles/{profile_id}.json: profile must declare required feedback families")
    for family in required_families:
      if family not in priority_map:
        issues.append(f"game/data/content/quality-profiles/{profile_id}.json: missing priority for feedback family {family}")
    if profile_id == "quality.phase7.playable":
      for field_name in [
        "input_to_motion_latency_budget_ms",
        "interaction_hitch_budget_ms",
        "audio_voice_concurrency_budget",
      ]:
        if not payload.get(field_name):
          issues.append(
            f"game/data/content/quality-profiles/{profile_id}.json: missing required Phase 7 field {field_name}"
          )

  return issues


def run_lint() -> int:
  lint_script = REPO_ROOT / "tools" / "content-validation" / "lint_repo.py"
  result = subprocess.run([sys.executable, str(lint_script)], cwd=REPO_ROOT)
  return result.returncode


def main() -> int:
  parser = argparse.ArgumentParser(description="Validate PeterCraft data contracts.")
  parser.add_argument("mode", choices=["full", "files", "changed"], help="Validation mode.")
  parser.add_argument("--include-lint", action="store_true", help="Run repository lint too.")
  parser.add_argument("--base-ref", default="origin/main", help="Base ref for changed mode.")
  parser.add_argument("files", nargs="*", help="Files to validate in files mode.")
  args = parser.parse_args()

  issues: list[str] = []

  if args.mode == "full":
    issues.extend(validate_expected_valid(sorted(EXAMPLES_ROOT.glob("*.json"))))
    issues.extend(validate_expected_valid(iter_content_files()))
    issues.extend(validate_expected_invalid(sorted(INVALID_ROOT.glob("*.json"))))
    issues.extend(validate_content_registry())
  elif args.mode == "files":
    if not args.files:
      print("files mode requires at least one file path")
      return 1
    selected = [REPO_ROOT / path for path in args.files]
    issues.extend(validate_expected_valid(selected))
    if any(CONTENT_ROOT in path.parents for path in selected):
      issues.extend(validate_content_registry())
  else:
    changed_files = collect_changed_files(args.base_ref)
    if changed_files:
      issues.extend(validate_expected_valid(changed_files))
      if any(CONTENT_ROOT in path.parents for path in changed_files):
        issues.extend(validate_content_registry())

  if issues:
    for issue in issues:
      print(issue)
    return 1

  if args.include_lint and run_lint() != 0:
    return 1

  print("Validation passed.")
  return 0


if __name__ == "__main__":
  sys.exit(main())
