from __future__ import annotations

import argparse
import pathlib
import sys


REPO_ROOT = pathlib.Path(__file__).resolve().parents[2]
TEXT_SUFFIXES = {
  ".cmake",
  ".cpp",
  ".cxx",
  ".h",
  ".hpp",
  ".json",
  ".md",
  ".ps1",
  ".py",
  ".txt",
  ".yaml",
  ".yml",
}
EXCLUDED_PARTS = {".git", ".venv", "out", "Saved", "Cache", "user", "User"}


def iter_text_files() -> list[pathlib.Path]:
  files: list[pathlib.Path] = []
  for path in REPO_ROOT.rglob("*"):
    if not path.is_file():
      continue
    if any(part in EXCLUDED_PARTS for part in path.parts):
      continue
    if path.name == "CMakeLists.txt" or path.suffix in TEXT_SUFFIXES:
      files.append(path)
  return files


def check_text_rules(path: pathlib.Path) -> list[str]:
  issues: list[str] = []
  text = None
  last_error: Exception | None = None
  for encoding in ("utf-8-sig", "utf-16", "cp1252"):
    try:
      text = path.read_text(encoding=encoding)
      break
    except UnicodeDecodeError as exc:
      last_error = exc
  if text is None:
    raise last_error if last_error is not None else UnicodeDecodeError("utf-8", b"", 0, 1, "unable to decode")
  relative = path.relative_to(REPO_ROOT)

  if "\t" in text:
    issues.append(f"{relative}: tabs are not allowed in tracked text files")

  if not text.endswith("\n"):
    issues.append(f"{relative}: file must end with a newline")

  if path.suffix != ".md":
    for line_number, line in enumerate(text.splitlines(), start=1):
      if line.rstrip(" ") != line:
        issues.append(f"{relative}:{line_number}: trailing whitespace is not allowed")

  return issues


def check_module_layout() -> list[str]:
  issues: list[str] = []
  modules_root = REPO_ROOT / "game" / "code" / "gems"
  required_entries = {"README.md", "CMakeLists.txt", "include", "src", "tests"}

  for module_dir in modules_root.iterdir():
    if not module_dir.is_dir():
      continue

    present = {entry.name for entry in module_dir.iterdir()}
    missing = sorted(required_entries.difference(present))
    if missing:
      issues.append(
        f"{module_dir.relative_to(REPO_ROOT)}: missing required entries: {', '.join(missing)}"
      )

  return issues


def main() -> int:
  parser = argparse.ArgumentParser(description="Lint PeterCraft repository conventions.")
  parser.parse_args()

  issues: list[str] = []
  for path in iter_text_files():
    issues.extend(check_text_rules(path))
  issues.extend(check_module_layout())

  if issues:
    for issue in issues:
      print(issue)
    return 1

  print("Repository lint passed.")
  return 0


if __name__ == "__main__":
  sys.exit(main())
