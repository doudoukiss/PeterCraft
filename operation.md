# PeterCraft Operation Guide

## Purpose

This runbook explains how to bootstrap, build, validate, test, and run PeterCraft in Phase 7.0.

The repo currently supports two named runtime modes:

- `headless`
  - the real runtime for Phase 7.0
  - used for development, tests, validation, deterministic scenarios, and CI
- `playable`
  - preflight-only in Phase 7.0
  - confirms the future engine-backed path is wired into the app and scripts
  - exits cleanly with a “backend unavailable until Phase 7.1” result

## Environment

- OS: Windows
- shell: PowerShell
- build system: CMake + Visual Studio 2022 presets
- Python: used for validation and quality tooling

## Prerequisites

Make sure the machine has:

- Visual Studio 2022 with the C++ desktop workload
- CMake 3.28 or newer on `PATH`
- Python 3.12 available through `py -3`
- Git
- Git LFS recommended

## First-time setup

From the repository root:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\bootstrap.ps1
```

This creates `.venv`, installs Python tooling, and enables local Git LFS support when available.

## Build the headless runtime

Canonical command:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\build-headless.ps1
```

Defaults:

- configure preset: `windows-vs2022-headless`
- build preset: `windows-vs2022-headless-debug`

Legacy wrapper:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\build.ps1
```

The executable is produced under:

- `out/build/windows-vs2022-headless/bin/Debug/PeterCraftApp.exe`

## Build the playable preflight

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\build-playable.ps1
```

What this means in Phase 7.0:

- builds the same app shell under the playable-preflight preset
- proves the build option and runtime-selection path exist
- does **not** build a real engine-backed runtime yet

## Validate content and contracts

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\validate.ps1
```

Useful modes:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\validate.ps1 -Mode changed
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\validate.ps1 -Mode files -Files game\data\content\quality-profiles\quality.phase7.playable.json
```

## Run automated tests

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\test.ps1
```

This runs unit, integration, scenario, headless smoke, and the playable-preflight-unavailable check.

## Run the headless runtime

Canonical command:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-headless.ps1
```

Defaults:

- runtime: `headless`
- profile: `player.default`
- scenario: `guided_first_run`
- configuration: `Debug`

Legacy wrapper:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-dev.ps1
```

## Run a specific headless scenario

Examples:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-headless.ps1 -Scenario guided_first_run -ProfileId player.ops.guided
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-headless.ps1 -Scenario happy_path -ProfileId player.ops.happy
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-headless.ps1 -Scenario failure_path -ProfileId player.ops.failure
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-headless.ps1 -Scenario artifact_recovery -ProfileId player.ops.artifact
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-headless.ps1 -Scenario escort_support -ProfileId player.ops.escort
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-headless.ps1 -Scenario smoke -ProfileId player.ops.smoke
```

## Run the playable preflight

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-playable.ps1
```

Expected Phase 7.0 result:

- the app selects `--runtime playable`
- the runtime exits cleanly
- the wrapper reports that the backend is unavailable until Phase 7.1

## Run the executable directly

Headless:

```powershell
.\out\build\windows-vs2022-headless\bin\Debug\PeterCraftApp.exe --runtime headless --profile-id player.direct.run --scenario guided_first_run
```

Playable preflight:

```powershell
.\out\build\windows-vs2022-playable-preflight\bin\Debug\PeterCraftApp.exe --runtime playable --profile-id player.direct.playable --scenario guided_first_run
```

## Quality checks

Phase 6 headless gate:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\quality.ps1 check-budgets --profile-id phase6_shell
```

Phase 7 playable dashboard:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\quality.ps1 check-budgets --profile-id phase7_playable
```

Save health:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\quality.ps1 verify-saves
```

Release-candidate gate:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\quality.ps1 gate-rc --profile-id phase6_shell
```

Other useful quality commands:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\quality.ps1 verify-profile --profile-id player.default
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\quality.ps1 run-soak --iterations 2
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\quality.ps1 export-qa-matrix
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\quality.ps1 summarize-playtest
```

## Output locations

Runtime and generated outputs are written under `Saved/`.

Important paths:

- logs: `Saved/Logs/`
- telemetry JSONL: `Saved/Logs/petercraft-events.jsonl`
- generated quality reports: `Saved/Generated/quality/`
- profiles and save data: `Saved/Profiles/`
- creator artifacts: `Saved/Profiles/<profile>/CreatorContent/`

## Typical daily flow

Use this sequence for normal development or QA:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\build-headless.ps1
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\validate.ps1
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\test.ps1
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-headless.ps1
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\quality.ps1 check-budgets --profile-id phase6_shell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\quality.ps1 check-budgets --profile-id phase7_playable
```

## Troubleshooting

If the app does not run:

1. Make sure `build-headless.ps1` completed successfully.
2. Make sure the executable exists at `out/build/windows-vs2022-headless/bin/Debug/PeterCraftApp.exe`.
3. Re-run `bootstrap.ps1` if Python tooling fails.
4. Re-run `validate.ps1` to catch data/schema issues.
5. Re-run `test.ps1` to confirm the headless shell is still healthy.

If the playable path does not behave as expected:

1. Run `build-playable.ps1`.
2. Run `run-playable.ps1`.
3. Confirm the result is a clean “backend unavailable until Phase 7.1” message rather than a crash.

If quality tools fail on a specific profile:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\quality.ps1 check-budgets --profile-id phase7_playable
```

If you want an isolated clean run, use a fresh profile id:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-headless.ps1 -ProfileId player.clean.run -Scenario guided_first_run
```

## Recommended operator checks

A run is considered healthy when:

- the headless app launches without crashing
- the selected scenario reaches a summary screen
- `test.ps1` is green
- `validate.ps1` is green
- `quality.ps1 check-budgets --profile-id phase6_shell` is green
- `quality.ps1 verify-saves` reports zero issues
- `run-playable.ps1` confirms the Phase 7.1 handoff cleanly
