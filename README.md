# PeterCraft

PeterCraft is a PC-first, single-player extraction-adventure maker game for children.
This repository now contains a runnable Phase 1 vertical slice on top of the Phase 0
foundation: portable gameplay contracts, validation, tests, save domains, telemetry, and
one deterministic home/raid/craft/explain/tune loop.

## Current slice goals

- Keep gameplay rules portable and data-driven.
- Separate engine adapters from game logic from day one.
- Provide one-command local workflows for bootstrap, build, test, validate, and run.
- Keep observability, validation, and debug support mandatory instead of optional.
- Prove one complete vertical slice loop before scaling content or systems.

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

- a deterministic vertical slice with one home base, one raid, one extraction flow, one craft, one explain panel, and one safe rule edit
- explicit save domains for profile meta, inventory, mission progress, tutorial progress, workshop upgrades, and companion config
- schema-driven content contracts for rooms, missions, encounters, extraction, crafting, carry capacity, home stations, and companion rules
- structured JSONL telemetry plus automated tests for happy path, failure path, persistence, and rule-edit behavior

It still keeps engine binding behind adapters and does not yet attempt content scale,
advanced procedural generation, or unrestricted creator tooling.
