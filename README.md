# PeterCraft

PeterCraft is a PC-first, single-player extraction-adventure maker game for children.
This repository now contains a runnable Phase 6 quality beta on top of the Phase 0
foundation, Phase 1 slice, and Phase 2 systems work: portable gameplay contracts, validation,
tests, save domains, telemetry, deterministic AI scenarios, safe creator content storage, and a
readable home/mission/raid loop.

## Current slice goals

- Keep gameplay rules portable and data-driven.
- Separate engine adapters from game logic from day one.
- Provide one-command local workflows for bootstrap, build, test, validate, and run.
- Keep observability, validation, and debug support mandatory instead of optional.
- Deepen the creator workshop with safe value, rule, tiny-script, and mini-mission layers.
- Scale shipped content through runtime-loaded catalogs, review records, and thin authoring tools.
- Harden saves, accessibility, onboarding evidence, and release-quality gates before the RC phase.

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
6. Run `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\quality.ps1 check-budgets`

## Current status

The current implementation includes:

- a deterministic AI alpha with utility-driven companion decisions, two enemy archetypes, five stances, five behavior chips, explainability panels, and six authored AI scenarios
- the existing five mission templates, richer combat events, item taxonomy and rarity rules, recovery/favorite-item handling, progression tracks, and authored tutorial lessons
- explicit save domains for profile meta, inventory, recovery state, mission progress, tutorial progress, workshop upgrades, companion config, and accessibility settings
- creator save domains and versioned profile-local creator artifacts under `Profiles/<profile>/CreatorContent/` for tinker presets, logic rules, tiny scripts, mini missions, and mentor reports
- schema-driven content contracts for items, missions, lessons, recovery rules, upgrade tracks, accessibility settings, companion chips, companion stances, enemy archetypes, patrol routes, AI scenarios, tinker variables, logic rules, tiny scripts, mini missions, and creator manifests
- structured JSONL telemetry plus automated tests for happy path, failure path, migration, persistence, artifact recovery, escort support, creator activation/rollback, replay compare output, and deterministic AI scenarios
- runtime-loaded Phase 5 shipped content catalogs under `game/data/content/` for room kits, room variants, encounter patterns, mission blueprints, feedback tags, style profiles, and the shipped content manifest
- three authored raid zones and six authored mission blueprints assembled from the portable catalogs instead of hardcoded mission definitions
- authoring support under `tools/content-authoring/` for scaffolding, previews, room metrics, content diffs, and generated preview artifacts
- review records for shippable rooms, encounters, and missions under `docs/quality/content-reviews/`
- Phase 6 quality tooling under `tools/quality/` for budgets, soak runs, save-health verification, QA exports, playtest summaries, and release-candidate gates
- atomic save-domain writes with backup restore paths plus creator-artifact health inspection and restore-by-revision support
- expanded accessibility settings, onboarding funnel telemetry, child-facing copy helpers, and release-quality debug/quality overlay output

It still keeps engine binding behind adapters and does not yet attempt content scale,
advanced procedural generation, or unrestricted creator tooling.
