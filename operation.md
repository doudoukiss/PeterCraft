# PeterCraft Operation Guide

## Purpose

This file explains how to bootstrap, build, validate, test, and run the current PeterCraft game build on a local Windows machine.

The current game is a Phase 6 portable runtime shell. It runs as a deterministic app with text-rendered UI/debug output and O3DE-ready adapter seams.

## Environment

- OS: Windows
- Shell: PowerShell
- Build system: CMake + Visual Studio 2022 presets
- Python: used for validation and quality tooling

## Prerequisites

Before running the game, make sure the machine has:

- Visual Studio 2022 with C++ build tools
- CMake available on `PATH`
- Python 3 available through `py -3`
- Git
- Git LFS recommended

## First-Time Setup

From the repository root:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\bootstrap.ps1
```

What this does:

- creates `.venv` if needed
- upgrades `pip`
- installs validation dependencies
- runs `git lfs install --local` when available

## Build The Game

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\build.ps1
```

This configures and builds the default Windows debug preset:

- configure preset: `windows-vs2022-dev`
- build preset: `windows-vs2022-dev-debug`

The built executable is produced under:

- `out/build/windows-vs2022-dev/bin/Debug/PeterCraftApp.exe`

## Validate Content And Contracts

Run full validation:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\validate.ps1
```

Useful modes:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\validate.ps1 -Mode changed
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\validate.ps1 -Mode files -Files game\data\schemas\accessibility_settings.schema.json
```

## Run Automated Tests

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\test.ps1
```

This runs:

- unit tests
- integration tests
- deterministic scenario tests
- slice smoke/path tests

## Run The Game

Default run:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-dev.ps1
```

Default run parameters:

- profile: `player.default`
- scenario: `guided_first_run`
- configuration: `Debug`

## Run A Specific Scenario

Examples:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-dev.ps1 -Scenario guided_first_run -ProfileId player.ops.guided
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-dev.ps1 -Scenario happy_path -ProfileId player.ops.happy
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-dev.ps1 -Scenario failure_path -ProfileId player.ops.failure
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-dev.ps1 -Scenario artifact_recovery -ProfileId player.ops.artifact
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-dev.ps1 -Scenario escort_support -ProfileId player.ops.escort
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-dev.ps1 -Scenario smoke -ProfileId player.ops.smoke
```

You can also pass raw app arguments through `-AppArguments`.

Example:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-dev.ps1 -ProfileId player.ops.custom -Scenario guided_first_run -AppArguments @('--no-settings')
```

## Run The Executable Directly

If you want to bypass the wrapper script:

```powershell
.\out\build\windows-vs2022-dev\bin\Debug\PeterCraftApp.exe --profile-id player.direct.run --scenario guided_first_run
```

## Quality Checks

Budget check:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\quality.ps1 check-budgets
```

Save health:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\quality.ps1 verify-saves
```

Release-candidate gate:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\quality.ps1 gate-rc
```

Other useful quality commands:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\quality.ps1 verify-profile --profile-id player.default
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\quality.ps1 run-soak --iterations 2
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\quality.ps1 export-qa-matrix
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\quality.ps1 summarize-playtest
```

## Output Locations

Runtime and generated outputs are written under `Saved/`.

Important paths:

- logs: `Saved/Logs/`
- telemetry JSONL: `Saved/Logs/petercraft-events.jsonl`
- generated quality reports: `Saved/Generated/quality/`
- profiles and save data: `Saved/Profiles/`
- creator artifacts: `Saved/Profiles/<profile>/CreatorContent/`

## Typical Daily Flow

Use this sequence for normal development or QA:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\build.ps1
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\validate.ps1
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\test.ps1
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-dev.ps1
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\quality.ps1 check-budgets
```

## Troubleshooting

If the app does not run:

1. Make sure `build.ps1` completed successfully.
2. Make sure the executable exists at `out/build/windows-vs2022-dev/bin/Debug/PeterCraftApp.exe`.
3. Re-run `bootstrap.ps1` if Python tooling or validation commands fail.
4. Re-run `validate.ps1` to catch data/schema issues.
5. Re-run `test.ps1` to confirm the runtime shell is still healthy.

If quality tools fail on a specific profile:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\quality.ps1 verify-profile --profile-id player.default
```

If you want an isolated clean run, use a fresh profile id:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-dev.ps1 -ProfileId player.clean.run -Scenario guided_first_run
```

## Recommended Operator Checks

A run is considered healthy when:

- the app launches without crashing
- the selected scenario reaches a summary screen
- `test.ps1` is green
- `validate.ps1` is green
- `quality.ps1 check-budgets` is green
- `quality.ps1 verify-saves` reports zero issues

