# PeterCraft Operation Guide

## Purpose

This runbook explains how to bootstrap, build, validate, test, and run PeterCraft in Phase 7.1.

The repo supports two runtime modes:

- `headless`
  - the stable default for development, tests, validation, deterministic scenarios, and CI
- `playable`
  - the Windows-only O3DE-backed runtime baseline in Phase 7.1
  - launches real levels from the in-repo O3DE project

## Environment

- OS: Windows 11 preferred
- shell: PowerShell
- build system: CMake + Visual Studio 2022
- Python: used for validation and quality tooling
- O3DE: pinned to `25.10.2`
- override: set `PETERCRAFT_O3DE_ROOT` if O3DE is not installed at `C:\o3de\25.10.2`

## Prerequisites

Make sure the machine has:

- Visual Studio 2022 with the C++ desktop workload
- CMake 3.28 or newer on `PATH`
- Python 3.12 available through `py -3`
- Git
- Git LFS recommended
- O3DE `25.10.2`

## First-time setup

From the repository root:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\bootstrap.ps1
```

## Headless workflow

Build:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\build-headless.ps1
```

Validate:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\validate.ps1
```

Test:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\test.ps1
```

Run:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-headless.ps1
```

Legacy wrappers:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\build.ps1
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-dev.ps1
```

## Playable workflow

Build the host app and O3DE project:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\build-playable.ps1
```

This script:

- registers the O3DE engine and PeterCraft project
- builds the portable host app with playable runtime support enabled
- configures and builds the O3DE project launchers
- runs `AssetProcessorBatch` for the playable project

Run the playable runtime:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-playable.ps1
```

Launch the one-room proof directly:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-playable.ps1 -LaunchOneRoomProof
```

## Direct executable usage

Headless:

```powershell
.\out\build\windows-vs2022-headless\bin\Debug\PeterCraftApp.exe --runtime headless --profile-id player.direct.run --scenario guided_first_run
```

Playable host:

```powershell
.\out\build\windows-vs2022-playable-runtime\bin\Debug\PeterCraftApp.exe --runtime playable --profile-id player.direct.playable --scenario guided_first_run
```

## Quality checks

Phase 6 shell gate:

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

## Output locations

- runtime logs: `Saved/Logs/`
- adapter log: `Saved/Logs/petercraft-o3de-adapter.log`
- telemetry JSONL: `Saved/Logs/petercraft-events.jsonl`
- generated quality reports: `Saved/Generated/quality/`
- generated playable evidence: `Saved/Generated/o3de/`
- profiles and save data: `Saved/Profiles/`
- creator artifacts: `Saved/Profiles/<profile>/CreatorContent/`

## Recommended daily flow

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\build-headless.ps1
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\validate.ps1
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\test.ps1
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-headless.ps1
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\build-playable.ps1
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-playable.ps1
```

## Troubleshooting

If the headless app does not run:

1. Re-run `build-headless.ps1`.
2. Re-run `validate.ps1`.
3. Re-run `test.ps1`.

If the playable path does not run:

1. Confirm O3DE `25.10.2` is installed or set `PETERCRAFT_O3DE_ROOT`.
2. Run `build-playable.ps1`.
3. Check `Saved/Logs/petercraft-o3de-adapter.log`.
4. Re-run `run-playable.ps1 -LaunchOneRoomProof`.

If scene loading fails:

1. Verify `game/data/content/scene-bindings/` points at existing level prefabs under `game/o3de/Levels/`.
2. Verify the O3DE project is registered.
3. Check the adapter log for launcher or registry-write failures.

## Healthy-run checklist

A run is considered healthy when:

- the headless app launches without crashing
- headless validation and tests are green
- the playable build completes
- `run-playable.ps1` launches the O3DE project without a bootstrap failure
- `run-playable.ps1 -LaunchOneRoomProof` opens the proof room
- quality and save-health checks stay green
