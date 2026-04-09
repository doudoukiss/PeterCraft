# Runtime Modes

## Purpose

Phase 7.0 makes PeterCraft's runtime split explicit before O3DE integration begins.

There are now two named runtime modes:

- `headless`
- `playable`

## Headless

Use `headless` for:

- local development
- validation
- automated tests
- deterministic scenarios
- CI smoke

Current backend:

- `NullPlatformServices`

Canonical commands:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\build-headless.ps1
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-headless.ps1
```

## Playable

Use `playable` only as a migration preflight in Phase 7.0.

Current behavior:

- the app accepts `--runtime playable`
- backend selection is explicit
- the runtime exits cleanly with a structured “backend unavailable until Phase 7.1” result

Canonical commands:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\build-playable.ps1
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-playable.ps1
```

## CLI contract

The executable now supports:

```text
--runtime headless|playable
```

Defaults:

- runtime defaults to `headless`
- tests and CI should stay on `headless` unless a specific preflight check needs `playable`

## Why this split matters

- it protects the current deterministic shell
- it makes the future O3DE path visible without pretending it exists yet
- it keeps build scripts, docs, and CI honest during Phase 7 migration
