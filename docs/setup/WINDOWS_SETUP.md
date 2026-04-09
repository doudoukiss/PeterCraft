# Windows Setup

## Required tools

- Windows 11 preferred
- Visual Studio 2022 with the C++ desktop workload
- CMake 3.28 or newer
- Python 3.12 available through the `py` launcher
- Git with Git LFS enabled
- O3DE `25.10.2`

## Runtime modes

- `headless` is the stable local and CI runtime
- `playable` is the Windows-only O3DE-backed runtime baseline in Phase 7.1

## First-time setup

1. Clone the repository.
2. Run `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\bootstrap.ps1`
3. Run `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\build-headless.ps1`
4. Run `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\validate.ps1`
5. Run `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\test.ps1`
6. Run `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-headless.ps1`
7. Run `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\build-playable.ps1`
8. Run `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-playable.ps1`

## O3DE location

Default engine root:

- `C:\o3de\25.10.2`

Override if needed:

```powershell
$env:PETERCRAFT_O3DE_ROOT='D:\tools\o3de\25.10.2'
```

## Notes

- Windows remains the required local path for the playable runtime.
- Headless stays authoritative for tests and CI in Phase 7.1.
- The playable path uses the in-repo project at `game/o3de/` and keeps core gameplay rules outside engine-only assets.
