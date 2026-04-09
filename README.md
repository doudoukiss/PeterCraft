# PeterCraft

PeterCraft is a PC-first, single-player extraction-adventure maker game for children.
This repository now contains a **Phase 7.0 headless foundation**: the portable runtime,
content catalogs, save/creator safety, quality gates, deterministic scenarios, and a
clean split between the current headless backend and the future playable runtime path.

## Current focus

- keep gameplay rules portable and data-driven
- preserve the headless shell for tests, validation, and CI
- introduce explicit `headless` vs `playable` runtime naming before engine work
- keep quality, save safety, and explainability mandatory while Phase 7 ramps up

## Repository layout

- `engine-adapters/` backend-facing adapter contracts and the current null/headless backend
- `game/` runtime code, gems, content catalogs, assets, UI/audio placeholders, and app shell
- `tools/` validation, build, quality, authoring, and migration utilities
- `tests/` unit, integration, scenario, and smoke coverage
- `docs/` ADRs, setup guides, execution boards, and quality/process docs

## Runtime modes

- `headless`
  - current default
  - used by tests, validation, deterministic scenarios, and CI smoke
  - runs the portable shell with `NullPlatformServices`
- `playable`
  - explicit preflight-only path in Phase 7.0
  - exits cleanly with “backend unavailable until Phase 7.1”
  - exists so the migration path is visible before O3DE integration starts

More detail is in [operation.md](operation.md) and [RUNTIME_MODES.md](docs/setup/RUNTIME_MODES.md).

## Quick start

1. Run `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\bootstrap.ps1`
2. Run `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\build-headless.ps1`
3. Run `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\validate.ps1`
4. Run `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\test.ps1`
5. Run `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-headless.ps1`
6. Run `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\quality.ps1 check-budgets --profile-id phase6_shell`

Legacy wrappers still exist:

- `build.ps1` calls the headless build path
- `run-dev.ps1` calls the headless run path

## Playable preflight

Phase 7.0 does not ship an engine-backed runtime yet, but the migration commands are now explicit:

- `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\build-playable.ps1`
- `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-playable.ps1`

`run-playable.ps1` should confirm that the backend is unavailable until Phase 7.1 instead of crashing or pretending the path exists.

## Current status

The repo currently includes:

- the Phase 0 through Phase 6 portable runtime, content, creator, save-safety, validation, and quality work
- explicit runtime selection with `--runtime headless|playable`
- Phase 7 migration flags surfaced through the debug overlay and runtime events
- separate Phase 6 shell and Phase 7 playable quality profiles
- headless-first Windows Debug/Release workflows plus Linux Clang compile smoke in CI
- explicit headless/playable local scripts so contributors do not have to guess which path to use

The repo still does **not** include the actual O3DE-backed playable runtime. That work starts in Phase 7.1.
