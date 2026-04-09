# Phase 7 Migration Board

## Build hardening

- Task: keep strict-warning Windows Debug and Release builds green
  - dependencies: Phase 7.0 runtime separation branch landed
  - acceptance: `PETERCRAFT_ENABLE_STRICT_WARNINGS=ON` passes in Debug and Release
  - repo area: `CMakeLists.txt`, `game/code/gems/*`, `.github/workflows/ci.yml`
- Task: keep non-Windows compile smoke green
  - dependencies: platform-specific linkage is guarded
  - acceptance: `linux-clang-headless-smoke` configure/build passes
  - repo area: `CMakePresets.json`, `.github/workflows/ci.yml`, `game/code/gems/PeterTelemetry/`

## Runtime separation

- Task: maintain `headless` as the default stable runtime
  - dependencies: runtime descriptor and backend factory exist
  - acceptance: tests, validation, and headless smoke all run with `--runtime headless`
  - repo area: `engine-adapters/`, `game/code/app/`, `tests/`, `tools/build-scripts/`
- Task: keep `playable` as a clean preflight-only path until Phase 7.1
  - dependencies: CLI/runtime selection is explicit
  - acceptance: `--runtime playable` exits cleanly with a backend-unavailable result
  - repo area: `game/code/app/`, `tools/build-scripts/run-playable.ps1`
- Task: replace the playable stub with an O3DE-backed bootstrap in Phase 7.1
  - dependencies: in-repo O3DE project, adapter seam, scene-binding catalog
  - acceptance: `--runtime playable` registers the project, creates O3DE platform services, and can launch the bound one-room proof
  - repo area: `engine-adapters/o3de/`, `game/o3de/`, `game/code/app/`, `game/code/gems/PeterWorld/`

## Engine integration

- Task: add the O3DE-backed platform-services stack in Phase 7.1
  - dependencies: runtime separation complete, quality budgets defined, CI stable
  - acceptance: backend selection can instantiate a real playable adapter stack
  - repo area: `engine-adapters/`, `game/code/app/`, `game/assets/`, `game/ui/`
- Task: document the O3DE baseline and decision-gate evidence
  - dependencies: pinned engine version, build scripts, runtime spike outputs
  - acceptance: contributors can install O3DE, build playable, run playable, debug adapter failures, and find the measurement dashboard without guessing
  - repo area: `docs/setup/`, `docs/adr/`, `docs/execution/`, `Saved/Generated/o3de/`

## Traversal

- Task: replace headless traversal presentation with real playable controller/camera behavior
  - dependencies: engine integration baseline landed
  - acceptance: over-the-shoulder traversal works in the playable runtime and retains comfort controls
  - repo area: `game/code/gems/PeterTraversal/`, `engine-adapters/`, `game/ui/`

## Combat

- Task: bring the portable combat rules into real-time world execution
  - dependencies: playable controller, interaction anchors, runtime HUD baseline
  - acceptance: one patrol fight is readable, fair, and instrumented
  - repo area: `game/code/gems/PeterCombat/`, `game/code/app/`, `engine-adapters/`

## AI

- Task: host companion and enemy decisions in the playable world while preserving explainability
  - dependencies: engine integration, traversal, combat feedback hooks
  - acceptance: the companion remains explainable in one real-time raid run
  - repo area: `game/code/gems/PeterAI/`, `game/code/gems/PeterDebug/`, `game/code/gems/PeterUI/`

## World/content

- Task: connect the existing mission blueprints and room content to one playable home and one playable raid
  - dependencies: engine-backed world loading and interaction hooks
  - acceptance: one mission can start, run, extract, and return home without breaking saves
  - repo area: `game/code/gems/PeterWorld/`, `game/data/`, `game/assets/`

## HUD/audio

- Task: convert current shell presentation into a readable playable HUD plus basic audio cues
  - dependencies: engine-backed UI and feedback adapters
  - acceptance: mission state, extraction countdown, low-health clarity, and basic cue routing are all present
  - repo area: `game/code/gems/PeterUI/`, `game/audio/`, `game/ui/`, `engine-adapters/`

## Quality/playtests

- Task: keep the Phase 6 headless gate green while standing up the Phase 7 playable dashboard
  - dependencies: separate Phase 7 quality profile and quality-tool support
  - acceptance: Phase 6 gates pass and Phase 7 rows render as measured or unmeasured intentionally
  - repo area: `game/code/gems/PeterCore/`, `game/code/gems/PeterTelemetry/`, `tools/quality/`
- Task: define Phase 7 acceptance checks before playable feature work expands
  - dependencies: runtime naming, scripts, and CI are explicit
  - acceptance: contributors can answer how to validate, build headless, run headless, build playable, and run playable without guessing
  - repo area: `README.md`, `operation.md`, `docs/setup/`, `docs/quality/`
