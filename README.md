# PeterCraft

PeterCraft is a PC-first, single-player extraction-adventure maker game for children.
This repository now contains a **Phase 7.1 playable runtime baseline**: the portable headless shell is still the authoritative test path, and the repo now also carries a real O3DE project plus a Windows-only playable bootstrap path.

## Current focus

- keep gameplay rules portable and data-driven
- preserve the headless shell for tests, validation, and CI
- host the runtime through O3DE without pushing core rules into engine-only scripts
- keep quality, save safety, explainability, and creator safety mandatory during playable integration

## Repository layout

- `engine-adapters/` adapter contracts, null backend, and O3DE-backed playable adapters
- `game/` runtime code, gems, content catalogs, and the in-repo O3DE project under `game/o3de/`
- `tools/` validation, build, quality, authoring, and migration utilities
- `tests/` unit, integration, scenario, and smoke coverage
- `docs/` ADRs, setup guides, execution boards, and quality/process docs

## Runtime modes

- `headless`
  - default and stable
  - used by tests, validation, deterministic scenarios, and CI smoke
  - runs the portable shell with `NullPlatformServices`
- `playable`
  - Windows-only Phase 7.1 path
  - bootstraps the pinned O3DE `25.10.2` project under `game/o3de/`
  - launches real O3DE levels through portable scene bindings while preserving the headless shell

More detail is in [operation.md](operation.md), [RUNTIME_MODES.md](docs/setup/RUNTIME_MODES.md), and [O3DE_SETUP.md](docs/setup/O3DE_SETUP.md).

## Quick start

1. `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\bootstrap.ps1`
2. `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\build-headless.ps1`
3. `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\validate.ps1`
4. `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\test.ps1`
5. `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-headless.ps1`
6. `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\build-playable.ps1`
7. `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-playable.ps1`

Legacy wrappers still exist:

- `build.ps1` calls the headless build path
- `run-dev.ps1` calls the headless run path

## Playable runtime

The playable flow is now a real Phase 7.1 bootstrap:

- registers the pinned O3DE engine root
- registers the in-repo project at `game/o3de/`
- builds the portable host app with playable runtime support enabled
- configures and builds the O3DE project launchers
- launches bound O3DE levels through the new scene-binding catalog

The one-room proof can be launched directly with:

- `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-playable.ps1 -LaunchOneRoomProof`

## Current status

The repo currently includes:

- the Phase 0 through Phase 6 portable runtime, content, creator, save-safety, validation, and quality work
- explicit runtime selection with `--runtime headless|playable`
- a Windows-only O3DE playable project pinned to `25.10.2`
- O3DE bootstrap, adapter logging, and real scene-binding based level launches
- separate Phase 6 shell and Phase 7 playable quality profiles
- headless-first Windows Debug/Release workflows plus Linux Clang compile smoke in CI
- additive manual playable smoke workflow expectations without making playable a PR prerequisite yet

The repo still does **not** include the later Phase 7.2+ real-time traversal/combat/HUD slice. Phase 7.1 is the engine-backed host milestone.
