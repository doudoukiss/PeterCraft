from __future__ import annotations

import argparse
import json
import pathlib
import statistics
import subprocess
import sys
from collections import Counter, defaultdict
from typing import Iterable


REPO_ROOT = pathlib.Path(__file__).resolve().parents[2]
SAVED_ROOT = REPO_ROOT / "Saved"
LOG_ROOT = SAVED_ROOT / "Logs"
GENERATED_ROOT = SAVED_ROOT / "Generated" / "quality"
QUALITY_PROFILE_PATHS = {
  "phase6_shell": REPO_ROOT / "game" / "data" / "content" / "quality-profiles" / "quality.phase6.shell.json",
  "phase7_playable": REPO_ROOT / "game" / "data" / "content" / "quality-profiles" / "quality.phase7.playable.json",
}
DOCS_ROOT = REPO_ROOT / "docs"
QUALITY_DOCS_ROOT = DOCS_ROOT / "quality"
PLAYTEST_ROOT = DOCS_ROOT / "playtests"


def load_quality_profile(profile_key: str) -> dict[str, object]:
  with QUALITY_PROFILE_PATHS[profile_key].open("r", encoding="utf-8-sig") as handle:
    return json.load(handle)


def ensure_generated_root() -> pathlib.Path:
  GENERATED_ROOT.mkdir(parents=True, exist_ok=True)
  return GENERATED_ROOT


def write_text(path: pathlib.Path, body: str) -> pathlib.Path:
  path.parent.mkdir(parents=True, exist_ok=True)
  path.write_text(body, encoding="utf-8")
  return path


def write_json_outputs(base_name: str, profile_key: str, payload: dict[str, object]) -> None:
  output_root = ensure_generated_root()
  encoded = json.dumps(payload, indent=2)
  write_text(output_root / f"{base_name}.json", encoded)
  write_text(output_root / f"{base_name}-{profile_key}.json", encoded)


def write_markdown_outputs(base_name: str, profile_key: str, body: str) -> None:
  output_root = ensure_generated_root()
  write_text(output_root / f"{base_name}.md", body)
  write_text(output_root / f"{base_name}-{profile_key}.md", body)


def load_events(path: pathlib.Path) -> tuple[list[dict[str, object]], int]:
  events: list[dict[str, object]] = []
  malformed_lines = 0
  if not path.exists():
    return events, malformed_lines
  with path.open("r", encoding="utf-8-sig") as handle:
    for line in handle:
      line = line.strip()
      if not line:
        continue
      try:
        payload = json.loads(line)
      except json.JSONDecodeError:
        malformed_lines += 1
        continue
      if isinstance(payload, dict):
        events.append(payload)
      else:
        malformed_lines += 1
  return events, malformed_lines


def percentile(values: Iterable[float], ratio: float) -> float:
  ordered = sorted(values)
  if not ordered:
    return 0.0
  index = min(len(ordered) - 1, max(0, int(round((len(ordered) - 1) * ratio))))
  return ordered[index]


def sample_metrics(events: list[dict[str, object]]) -> dict[str, list[float]]:
  metrics: dict[str, list[float]] = defaultdict(list)
  for event in events:
    name = str(event.get("name", ""))
    fields = event.get("fields", {})
    if not isinstance(fields, dict):
      continue

    if name == "performance.boot.complete":
      metrics["cold_boot_ms"].append(float(fields.get("value", 0.0)))
    elif name == "performance.scene.transition":
      metrics["transition_ms"].append(float(fields.get("value", 0.0)))
    elif name == "performance.ai.decision":
      metrics["ai_decision_p95_ms"].append(float(fields.get("value", 0.0)))
    elif name == "performance.ui.panel_render":
      metrics["ui_render_p95_ms"].append(float(fields.get("value", 0.0)))
    elif name == "performance.memory.snapshot":
      metrics["working_set_mb"].append(float(fields.get("value", 0.0)))
    elif name == "performance.feedback.world_cue":
      metrics["concurrent_world_feedback"].append(float(fields.get("value", 0.0)))
    elif name == "performance.feedback.critical_beat":
      metrics["critical_feedback_per_beat"].append(float(fields.get("value", 0.0)))
    elif name == "performance.save.full_load":
      metrics["full_load_ms"].append(float(fields.get("value", 0.0)))
    elif name == "performance.save.full_save":
      metrics["full_save_ms"].append(float(fields.get("value", 0.0)))
    elif name == "performance.shell.frame_snapshot":
      metrics["frame_time_p95_ms"].append(float(fields.get("frame_time_ms", 0.0)))
      metrics["fps_average"].append(float(fields.get("fps", 0.0)))
    elif name == "save_load.domain.write":
      metrics["single_save_ms"].append(float(fields.get("duration_ms", 0.0)))
    elif name == "performance.input_to_motion":
      metrics["input_to_motion_latency_ms"].append(float(fields.get("value", fields.get("latency_ms", 0.0))))
    elif name == "performance.interaction.hitch":
      metrics["interaction_hitch_p95_ms"].append(float(fields.get("value", fields.get("duration_ms", 0.0))))
    elif name == "performance.audio.voice_concurrency":
      metrics["audio_voice_concurrency"].append(float(fields.get("value", fields.get("voices", 0.0))))
  return metrics


def aggregate_metric(metric_id: str, values: list[float]) -> float:
  if not values:
    return 0.0
  if metric_id == "fps_average":
    return statistics.fmean(values)
  if metric_id in {
    "frame_time_p95_ms",
    "ai_decision_p95_ms",
    "ui_render_p95_ms",
    "single_save_ms",
    "transition_ms",
    "input_to_motion_latency_ms",
    "interaction_hitch_p95_ms",
  }:
    return percentile(values, 0.95)
  if metric_id in {
    "concurrent_world_feedback",
    "critical_feedback_per_beat",
    "working_set_mb",
    "cold_boot_ms",
    "full_load_ms",
    "full_save_ms",
    "audio_voice_concurrency",
  }:
    return max(values)
  return statistics.fmean(values)


def budget_thresholds(profile: dict[str, object]) -> dict[str, float]:
  field_map = {
    "fps_average": "fps_target",
    "frame_time_p95_ms": "frame_time_p95_ms",
    "cold_boot_ms": "cold_boot_budget_ms",
    "transition_ms": "transition_budget_ms",
    "working_set_mb": "peak_working_set_budget_mb",
    "input_to_motion_latency_ms": "input_to_motion_latency_budget_ms",
    "ai_decision_p95_ms": "ai_decision_p95_ms",
    "ui_render_p95_ms": "ui_panel_render_p95_ms",
    "interaction_hitch_p95_ms": "interaction_hitch_budget_ms",
    "single_save_ms": "single_save_budget_ms",
    "full_load_ms": "full_load_budget_ms",
    "full_save_ms": "full_save_budget_ms",
    "audio_voice_concurrency": "audio_voice_concurrency_budget",
    "concurrent_world_feedback": "feedback_max_concurrent_world_cues",
    "critical_feedback_per_beat": "feedback_max_critical_cues_per_beat",
  }
  thresholds: dict[str, float] = {}
  for metric_id, field_name in field_map.items():
    if field_name in profile:
      thresholds[metric_id] = float(profile[field_name])
  return thresholds


def metric_passes(metric_id: str, value: float, threshold: float) -> bool:
  if metric_id == "fps_average":
    return value >= threshold
  return value <= threshold


def run_budget_check(telemetry_path: pathlib.Path, profile_key: str) -> int:
  profile = load_quality_profile(profile_key)
  profile_id = str(profile.get("id", profile_key))
  profile_name = str(profile.get("display_name", profile_id))
  thresholds = budget_thresholds(profile)
  events, malformed_lines = load_events(telemetry_path)
  metrics = sample_metrics(events)

  rows: list[dict[str, object]] = []
  passed = True
  unmeasured_count = 0
  for metric_id, threshold in thresholds.items():
    values = metrics.get(metric_id, [])
    measured = bool(values)
    aggregate = aggregate_metric(metric_id, values) if measured else None
    ok = metric_passes(metric_id, aggregate, threshold) if measured and aggregate is not None else True
    rows.append({
      "metric_id": metric_id,
      "value": aggregate,
      "budget": threshold,
      "measured": measured,
      "passed": ok,
      "samples": len(values),
      "status": "unmeasured" if not measured else ("pass" if ok else "fail"),
    })
    if not measured:
      unmeasured_count += 1
    if measured and not ok:
      passed = False

  payload = {
    "profile_key": profile_key,
    "profile_id": profile_id,
    "profile_name": profile_name,
    "malformed_lines": malformed_lines,
    "unmeasured_count": unmeasured_count,
    "passed": passed,
    "rows": rows,
  }
  write_json_outputs("budget-check", profile_key, payload)

  markdown = [
    "# Budget Check",
    f"- profile: `{profile_name}` (`{profile_id}`)",
    f"- telemetry: `{telemetry_path}`",
    f"- passed: `{str(passed).lower()}`",
    f"- unmeasured_metrics: `{unmeasured_count}`",
    f"- malformed_lines_skipped: `{malformed_lines}`",
    "",
  ]
  for row in rows:
    if row["measured"]:
      markdown.append(
        f"- `{row['metric_id']}` value={row['value']:.2f} budget={row['budget']:.2f} samples={row['samples']} status={row['status']}"
      )
    else:
      markdown.append(
        f"- `{row['metric_id']}` value=unmeasured budget={row['budget']:.2f} samples=0 status=unmeasured"
      )
  body = "\n".join(markdown) + "\n"
  write_markdown_outputs("budget-check", profile_key, body)

  print("\n".join(markdown))
  return 0 if passed else 1


def read_structured_json(path: pathlib.Path) -> tuple[bool, str]:
  try:
    payload = json.loads(path.read_text(encoding="utf-8-sig"))
  except Exception as exc:  # noqa: BLE001
    return False, str(exc)
  return isinstance(payload, dict), "ok"


def verify_profiles(profiles_root: pathlib.Path, repair_dry_run: bool = False) -> dict[str, object]:
  report: dict[str, object] = {
    "profiles_checked": 0,
    "save_files_checked": 0,
    "creator_files_checked": 0,
    "issues": [],
    "repair_dry_run": repair_dry_run,
  }

  if not profiles_root.exists():
    return report

  if (profiles_root / "SaveData").exists():
    profile_dirs = [profiles_root]
  else:
    profile_dirs = sorted(path for path in profiles_root.iterdir() if path.is_dir())

  for profile_dir in profile_dirs:
    report["profiles_checked"] += 1
    save_dir = profile_dir / "SaveData"
    creator_dir = profile_dir / "CreatorContent"
    backup_dir = profile_dir / "Backups"

    for path in sorted(save_dir.glob("*.json")):
      report["save_files_checked"] += 1
      valid, message = read_structured_json(path)
      latest_backup = backup_dir / f"{path.stem}.latest.bak.json"
      previous_backup = backup_dir / f"{path.stem}.previous.bak.json"
      if not valid:
        report["issues"].append({
          "kind": "save",
          "path": str(path.relative_to(REPO_ROOT)),
          "message": message,
          "latest_backup_exists": latest_backup.exists(),
          "previous_backup_exists": previous_backup.exists(),
          "repair_dry_run": repair_dry_run,
        })

    if creator_dir.exists():
      for path in sorted(creator_dir.rglob("*.json")):
        report["creator_files_checked"] += 1
        valid, message = read_structured_json(path)
        if not valid:
          report["issues"].append({
            "kind": "creator",
            "path": str(path.relative_to(REPO_ROOT)),
            "message": message,
            "repair_dry_run": repair_dry_run,
          })

  return report


def command_verify_saves(profiles_root: pathlib.Path) -> int:
  report = verify_profiles(profiles_root)
  output_root = ensure_generated_root()
  write_text(output_root / "save-health.json", json.dumps(report, indent=2))
  markdown = [
    "# Save Health",
    f"- profiles_checked: `{report['profiles_checked']}`",
    f"- save_files_checked: `{report['save_files_checked']}`",
    f"- creator_files_checked: `{report['creator_files_checked']}`",
    f"- issues: `{len(report['issues'])}`",
    "",
  ]
  for issue in report["issues"]:
    markdown.append(f"- `{issue['kind']}` `{issue['path']}`: {issue['message']}")
  write_text(output_root / "save-health.md", "\n".join(markdown) + "\n")
  print("\n".join(markdown))
  return 0 if not report["issues"] else 1


def command_verify_profile(profile_id: str, profiles_root: pathlib.Path) -> int:
  report = verify_profiles(profiles_root / profile_id)
  output_root = ensure_generated_root()
  write_text(output_root / f"verify-profile-{profile_id}.json", json.dumps(report, indent=2))
  print(json.dumps(report, indent=2))
  return 0 if not report["issues"] else 1


def command_repair_profile(profile_id: str, profiles_root: pathlib.Path, dry_run: bool) -> int:
  target = profiles_root / profile_id
  report = verify_profiles(target, repair_dry_run=dry_run)
  output_root = ensure_generated_root()
  suffix = "dry-run" if dry_run else "execute"
  write_text(output_root / f"repair-profile-{profile_id}-{suffix}.json", json.dumps(report, indent=2))
  print(json.dumps(report, indent=2))
  return 0 if not report["issues"] else 1


def command_diff_save_revisions(path_a: pathlib.Path, path_b: pathlib.Path) -> int:
  payload_a = json.loads(path_a.read_text(encoding="utf-8-sig"))
  payload_b = json.loads(path_b.read_text(encoding="utf-8-sig"))
  keys = sorted(set(payload_a) | set(payload_b))
  lines = ["# Save Revision Diff", f"- a: `{path_a}`", f"- b: `{path_b}`", ""]
  for key in keys:
    value_a = payload_a.get(key, "<missing>")
    value_b = payload_b.get(key, "<missing>")
    if value_a != value_b:
      lines.append(f"- `{key}`: `{value_a}` -> `{value_b}`")
  output_path = ensure_generated_root() / "save-diff.md"
  write_text(output_path, "\n".join(lines) + "\n")
  print("\n".join(lines))
  return 0


def command_export_qa_matrix() -> int:
  tests = [
    "validate.ps1",
    "build-headless.ps1",
    "run-headless.ps1",
    "build-playable.ps1",
    "run-playable.ps1 -Scenario smoke",
    "run-playable.ps1 -LaunchOneRoomProof",
    "test.ps1",
    "tools/quality/peter_quality.py check-budgets --profile-id phase6_shell",
    "tools/quality/peter_quality.py check-budgets --profile-id phase7_playable",
    "tools/quality/peter_quality.py verify-saves",
  ]
  matrix = [
    "# QA Matrix Export",
    "- platform: Windows PC",
    "- runtime_modes: headless default, playable O3DE-backed session loop",
    "- input_modes: mouse_keyboard, controller",
    "- save_cases: migration, backup restore, creator containment, invalid creator content",
    "- scenarios: happy_path, failure_path, artifact_recovery, escort_support, smoke, playable_smoke",
    "- creator_flows: preview, apply, revert, reset, script validation, mini mission launch",
    "- manual_review_docs: QA_MATRIX.md, ACCESSIBILITY_CHECKLIST.md, BUG_TRIAGE_RULES.md, RELEASE_CANDIDATE_POLICY.md",
    "",
    "## Automated Commands",
  ]
  matrix.extend(f"- `{command}`" for command in tests)
  output_path = ensure_generated_root() / "qa-matrix.md"
  write_text(output_path, "\n".join(matrix) + "\n")
  print("\n".join(matrix))
  return 0


def command_summarize_playtest(input_path: pathlib.Path) -> int:
  text = input_path.read_text(encoding="utf-8") if input_path.exists() else ""
  counter = Counter()
  for line in text.splitlines():
    stripped = line.strip().lower()
    if stripped.startswith("- confusion"):
      counter["confusion"] += 1
    elif stripped.startswith("- delight"):
      counter["delight"] += 1
    elif stripped.startswith("- friction"):
      counter["friction"] += 1
    elif stripped.startswith("- tutorial"):
      counter["tutorial"] += 1
    elif stripped.startswith("- workshop"):
      counter["workshop"] += 1

  lines = ["# Playtest Summary", f"- source: `{input_path}`", ""]
  for key in ["confusion", "delight", "friction", "tutorial", "workshop"]:
    lines.append(f"- {key}: `{counter.get(key, 0)}`")
  output_path = ensure_generated_root() / "playtest-summary.md"
  write_text(output_path, "\n".join(lines) + "\n")
  print("\n".join(lines))
  return 0


def command_run_soak(executable: pathlib.Path, iterations: int) -> int:
  scenarios = ["happy_path", "failure_path", "artifact_recovery", "escort_support"]
  output_root = ensure_generated_root()
  runs: list[dict[str, object]] = []
  for index in range(iterations):
    scenario = scenarios[index % len(scenarios)]
    profile_id = f"player.soak.{index:02d}"
    command = [
      str(executable),
      "--runtime",
      "headless",
      "--scenario",
      scenario,
      "--profile-id",
      profile_id,
      "--no-settings",
    ]
    result = subprocess.run(command, cwd=REPO_ROOT, capture_output=True, text=True)
    runs.append({
      "iteration": index + 1,
      "scenario": scenario,
      "returncode": result.returncode,
      "stdout_tail": "\n".join(result.stdout.splitlines()[-8:]),
    })
    if result.returncode != 0:
      break

  write_text(output_root / "soak-report.json", json.dumps(runs, indent=2))
  lines = ["# Soak Report", f"- executable: `{executable}`", f"- iterations_requested: `{iterations}`", ""]
  for run in runs:
    lines.append(f"- iter={run['iteration']} scenario={run['scenario']} returncode={run['returncode']}")
  write_text(output_root / "soak-report.md", "\n".join(lines) + "\n")
  print("\n".join(lines))
  return 0 if all(run["returncode"] == 0 for run in runs) and len(runs) == iterations else 1


def command_gate_rc(telemetry_path: pathlib.Path, profiles_root: pathlib.Path, profile_key: str) -> int:
  required_docs = [
    QUALITY_DOCS_ROOT / "QA_MATRIX.md",
    QUALITY_DOCS_ROOT / "ACCESSIBILITY_CHECKLIST.md",
    QUALITY_DOCS_ROOT / "BUG_TRIAGE_RULES.md",
    QUALITY_DOCS_ROOT / "RELEASE_CANDIDATE_POLICY.md",
    PLAYTEST_ROOT / "PLAYTEST_TEMPLATE.md",
    PLAYTEST_ROOT / "FINDINGS_LOG.md",
  ]
  missing_docs = [path for path in required_docs if not path.exists()]

  budget_code = run_budget_check(telemetry_path, profile_key)
  save_code = command_verify_saves(profiles_root)
  output_root = ensure_generated_root()
  lines = [
    "# RC Gate",
    f"- quality_profile: `{profile_key}`",
    f"- missing_docs: `{len(missing_docs)}`",
    f"- budget_gate: `{budget_code == 0}`",
    f"- save_gate: `{save_code == 0}`",
    "",
  ]
  for path in missing_docs:
    lines.append(f"- missing: `{path.relative_to(REPO_ROOT)}`")
  write_text(output_root / "rc-gate.md", "\n".join(lines) + "\n")
  print("\n".join(lines))
  return 0 if budget_code == 0 and save_code == 0 and not missing_docs else 1


def main() -> int:
  parser = argparse.ArgumentParser(description="Run PeterCraft quality tooling.")
  subparsers = parser.add_subparsers(dest="command", required=True)

  budget_parser = subparsers.add_parser("check-budgets")
  budget_parser.add_argument("--telemetry", type=pathlib.Path, default=LOG_ROOT / "petercraft-events.jsonl")
  budget_parser.add_argument("--profile-id", choices=sorted(QUALITY_PROFILE_PATHS), default="phase6_shell")

  verify_parser = subparsers.add_parser("verify-saves")
  verify_parser.add_argument("--profiles-root", type=pathlib.Path, default=SAVED_ROOT / "Profiles")

  verify_profile_parser = subparsers.add_parser("verify-profile")
  verify_profile_parser.add_argument("--profile-id", required=True)
  verify_profile_parser.add_argument("--profiles-root", type=pathlib.Path, default=SAVED_ROOT / "Profiles")

  repair_profile_parser = subparsers.add_parser("repair-profile")
  repair_profile_parser.add_argument("--profile-id", required=True)
  repair_profile_parser.add_argument("--profiles-root", type=pathlib.Path, default=SAVED_ROOT / "Profiles")
  repair_profile_parser.add_argument("--dry-run", action="store_true")

  diff_parser = subparsers.add_parser("diff-save-revisions")
  diff_parser.add_argument("--a", type=pathlib.Path, required=True)
  diff_parser.add_argument("--b", type=pathlib.Path, required=True)

  soak_parser = subparsers.add_parser("run-soak")
  soak_parser.add_argument(
    "--executable",
    type=pathlib.Path,
    default=REPO_ROOT / "out" / "build" / "windows-vs2022-headless" / "bin" / "Debug" / "PeterCraftApp.exe")
  soak_parser.add_argument("--iterations", type=int, default=4)

  subparsers.add_parser("export-qa-matrix")

  playtest_parser = subparsers.add_parser("summarize-playtest")
  playtest_parser.add_argument("--input", type=pathlib.Path, default=PLAYTEST_ROOT / "FINDINGS_LOG.md")

  gate_parser = subparsers.add_parser("gate-rc")
  gate_parser.add_argument("--telemetry", type=pathlib.Path, default=LOG_ROOT / "petercraft-events.jsonl")
  gate_parser.add_argument("--profiles-root", type=pathlib.Path, default=SAVED_ROOT / "Profiles")
  gate_parser.add_argument("--profile-id", choices=sorted(QUALITY_PROFILE_PATHS), default="phase6_shell")

  args = parser.parse_args()

  if args.command == "check-budgets":
    return run_budget_check(args.telemetry, args.profile_id)
  if args.command == "verify-saves":
    return command_verify_saves(args.profiles_root)
  if args.command == "verify-profile":
    return command_verify_profile(args.profile_id, args.profiles_root)
  if args.command == "repair-profile":
    return command_repair_profile(args.profile_id, args.profiles_root, args.dry_run)
  if args.command == "diff-save-revisions":
    return command_diff_save_revisions(args.a, args.b)
  if args.command == "run-soak":
    return command_run_soak(args.executable, args.iterations)
  if args.command == "export-qa-matrix":
    return command_export_qa_matrix()
  if args.command == "summarize-playtest":
    return command_summarize_playtest(args.input)
  if args.command == "gate-rc":
    return command_gate_rc(args.telemetry, args.profiles_root, args.profile_id)

  return 1


if __name__ == "__main__":
  sys.exit(main())
