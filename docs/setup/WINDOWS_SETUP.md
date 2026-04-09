# Windows Setup

## Required tools

- Windows 11 or Windows 10
- Visual Studio 2022 with C++ desktop workload
- CMake 3.28 or newer
- Python 3.12 available through the `py` launcher
- Git with Git LFS enabled

## Runtime modes

- `headless` is the current working runtime in Phase 7.0.
- `playable` is a preflight-only path in Phase 7.0 and should exit cleanly with a Phase 7.1 handoff message.

## First-time setup

1. Clone the repository.
2. Run `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\bootstrap.ps1`
3. Run `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\build-headless.ps1`
4. Run `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\validate.ps1`
5. Run `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\test.ps1`
6. Run `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-headless.ps1`
7. Optional: run `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\build-playable.ps1`
8. Optional: run `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-playable.ps1`

## Notes

- Windows remains the required local path for Phase 7.0.
- The current repo keeps engine integration behind adapter seams while the portable headless gameplay loop remains buildable and testable locally.
- GitHub Actions uses the same headless scripts as local development, plus a Linux Clang compile smoke path.
