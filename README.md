# PeterCraft

PeterCraft is a PC-first, single-player extraction-adventure maker game for children.
This repository contains the Phase 0 foundation: a portable gameplay skeleton, tooling,
validation, tests, and a bootable empty runtime shell that future O3DE integration can
attach to without rewriting core contracts.

## Phase 0 goals

- Keep gameplay rules portable and data-driven.
- Separate engine adapters from game logic from day one.
- Provide one-command local workflows for bootstrap, build, test, validate, and run.
- Keep observability, validation, and debug support mandatory instead of optional.

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

Phase 0 intentionally stops at:

- a buildable module skeleton
- a structured event bus and JSONL telemetry sink
- schema-driven example data with automated validation
- an empty runtime shell with placeholder menu and scene flow
- development overlays for key shell state

It does not yet include production gameplay, final UI, combat, companion behavior, or
content-heavy levels.
