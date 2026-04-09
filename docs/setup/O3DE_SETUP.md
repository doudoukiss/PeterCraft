# O3DE Setup

## Scope

This guide covers the Phase 7.1 playable runtime baseline.

Pinned engine version:

- `25.10.2`

Default engine root:

- `C:\o3de\25.10.2`

Override variable:

- `PETERCRAFT_O3DE_ROOT`

Project root:

- `game/o3de/`

## Install O3DE

Install O3DE `25.10.2` locally and make sure these files exist:

- `C:\o3de\25.10.2\scripts\o3de.bat`
- `C:\o3de\25.10.2\bin\Windows\profile\Default\O3DE.GameLauncher.exe`
- `C:\o3de\25.10.2\bin\Windows\profile\Default\Editor.exe`
- `C:\o3de\25.10.2\bin\Windows\profile\Default\AssetProcessorBatch.exe`

If your install lives elsewhere:

```powershell
$env:PETERCRAFT_O3DE_ROOT='D:\tools\o3de\25.10.2'
```

## Register the engine and project

The build scripts do this automatically, but the explicit commands are:

```powershell
& "$env:PETERCRAFT_O3DE_ROOT\scripts\o3de.bat" register --engine-path $env:PETERCRAFT_O3DE_ROOT --force
& "$env:PETERCRAFT_O3DE_ROOT\scripts\o3de.bat" register --project-path "$PWD\game\o3de" --engine-path $env:PETERCRAFT_O3DE_ROOT --force
```

The scripts also write `game/o3de/user/project.json` so the project resolves the pinned engine cleanly.

## Build the playable runtime

From the repo root:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\build-playable.ps1
```

This does four things:

1. registers the engine and project
2. builds the portable host app with `PETERCRAFT_ENABLE_PLAYABLE_RUNTIME=ON`
3. configures and builds the O3DE project launchers
4. runs `AssetProcessorBatch` for the project
5. ensures the local firewall rules needed for the launcher and asset processor

## Run the playable runtime

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-playable.ps1
```

## Run the one-room proof

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-playable.ps1 -LaunchOneRoomProof
```

## Scene bindings

Logical scenes stay portable in `game/data/content/scene-bindings/`.

Current Phase 7.1 bindings:

- `scene.playable.one_room_proof`
- `scene.home.workshop_yard`
- `scene.raid.machine_silo`
- `scene.results.success`
- `scene.results.failure`

These map into real O3DE levels under `game/o3de/Levels/`.

## Adapter failure debugging

Primary log:

- `Saved/Logs/petercraft-o3de-adapter.log`

Useful checks:

1. confirm the engine root exists
2. confirm `game/o3de/project.json` exists
3. confirm the launcher/editor/asset processor executables exist
4. confirm `game/data/content/scene-bindings/` points at real level prefabs
5. rerun `run-playable.ps1 -LaunchOneRoomProof`

## Re-run the proof room fast

After the initial build:

```powershell
powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-playable.ps1 -LaunchOneRoomProof
```
