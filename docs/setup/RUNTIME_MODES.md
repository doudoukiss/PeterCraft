# Runtime Modes

## Purpose

PeterCraft now keeps two first-class runtime modes:

- `headless`
- `playable`

This split protects the portable shell while allowing real O3DE hosting work to land incrementally.

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

Use `playable` for the Windows-only O3DE-backed playable session loop in Phase 7.2.

Current behavior:

- the app accepts `--runtime playable`
- the runtime bootstraps the pinned O3DE `25.10.2` project under `game/o3de/`
- logical scene requests are mapped through the portable scene-binding catalog
- the adapter layer launches real O3DE levels while the playable session controller drives traversal, interactions, extraction, and return-home flow

Canonical commands:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\build-playable.ps1
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-playable.ps1
```

One-room proof:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-playable.ps1 -LaunchOneRoomProof
```

## CLI contract

The executable supports:

```text
--runtime headless|playable
```

Defaults:

- runtime defaults to `headless`
- tests and CI stay on `headless`
- playable smoke remains additive in Phase 7.2

## Why this split matters

- it protects the deterministic shell
- it keeps O3DE-specific work behind adapters
- it lets PeterCraft grow a real playable slice without collapsing the headless reference path
