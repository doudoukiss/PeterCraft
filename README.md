# PeterCraft

PeterCraft is a PC-first, single-player extraction-adventure maker game for children.
This repository now contains a runnable Phase 3 AI-and-companions alpha on top of the Phase 0
foundation, Phase 1 slice, and Phase 2 systems work: portable gameplay contracts, validation,
tests, save domains, telemetry, deterministic AI scenarios, and a readable home/mission/raid loop.

## Current slice goals

- Keep gameplay rules portable and data-driven.
- Separate engine adapters from game logic from day one.
- Provide one-command local workflows for bootstrap, build, test, validate, and run.
- Keep observability, validation, and debug support mandatory instead of optional.
- Deepen the AI and companion layer before scaling content or creator complexity.

## Repository layout

- `engine-adapters/` engine-facing adapter contracts and null implementations
- `game/` runtime code, gems, assets, data, UI, and audio placeholders
- `tools/` validation, build, telemetry, and migration utilities
- `tests/` integration, scenario, and smoke harnesses
- `docs/` ADRs, setup guides, quality templates, and planning docs

## Quick start

1. Run `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\bootstrap.ps1`
2. Run `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\build.ps1`
3. Run `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\validate.ps1`
4. Run `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\test.ps1`
5. Run `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-dev.ps1`

## Current status

The current implementation includes:

- a deterministic AI alpha with utility-driven companion decisions, two enemy archetypes, five stances, five behavior chips, explainability panels, and six authored AI scenarios
- the existing five mission templates, richer combat events, item taxonomy and rarity rules, recovery/favorite-item handling, progression tracks, and authored tutorial lessons
- explicit save domains for profile meta, inventory, recovery state, mission progress, tutorial progress, workshop upgrades, companion config, and accessibility settings
- schema-driven content contracts for items, missions, lessons, recovery rules, upgrade tracks, accessibility settings, companion chips, companion stances, enemy archetypes, patrol routes, and AI scenarios
- structured JSONL telemetry plus automated tests for happy path, failure path, migration, persistence, artifact recovery, escort support, behavior-edit continuity, and deterministic AI scenarios

It still keeps engine binding behind adapters and does not yet attempt content scale,
advanced procedural generation, or unrestricted creator tooling.
