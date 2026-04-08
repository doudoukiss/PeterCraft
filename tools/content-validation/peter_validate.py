from __future__ import annotations

import argparse
import json
import pathlib
import subprocess
import sys
from typing import Iterable

from jsonschema import Draft202012Validator


REPO_ROOT = pathlib.Path(__file__).resolve().parents[2]
SCHEMA_ROOT = REPO_ROOT / "game" / "data" / "schemas"
EXAMPLES_ROOT = REPO_ROOT / "game" / "data" / "examples"
INVALID_ROOT = REPO_ROOT / "game" / "data" / "fixtures" / "invalid"


def schema_for_json(path: pathlib.Path) -> pathlib.Path:
  token = path.name.split(".")[0]
  schema_path = SCHEMA_ROOT / f"{token}.schema.json"
  if not schema_path.exists():
    raise FileNotFoundError(f"No schema found for {path.relative_to(REPO_ROOT)}")
  return schema_path


def load_json(path: pathlib.Path) -> object:
  with path.open("r", encoding="utf-8") as handle:
    return json.load(handle)


def validate_file(path: pathlib.Path) -> list[str]:
  schema = load_json(schema_for_json(path))
  payload = load_json(path)
  validator = Draft202012Validator(schema)

  return [
    f"{path.relative_to(REPO_ROOT)}: {error.message}"
    for error in validator.iter_errors(payload)
  ]


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
    issues.extend(validate_expected_invalid(sorted(INVALID_ROOT.glob("*.json"))))
  elif args.mode == "files":
    if not args.files:
      print("files mode requires at least one file path")
      return 1
    issues.extend(validate_expected_valid(REPO_ROOT / path for path in args.files))
  else:
    changed_files = collect_changed_files(args.base_ref)
    if changed_files:
      issues.extend(validate_expected_valid(changed_files))

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
