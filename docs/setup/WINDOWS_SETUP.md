# Windows Setup

## Required tools

- Windows 11 or Windows 10
- Visual Studio 2022 with C++ desktop workload
- CMake 3.28 or newer
- Python 3.12 available through the `py` launcher
- Git with Git LFS enabled

## First-time setup

1. Clone the repository.
2. Run `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\bootstrap.ps1`
3. Run `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\build.ps1`
4. Run `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\validate.ps1`
5. Run `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\test.ps1`
6. Run `powershell -ExecutionPolicy Bypass -File .\tools\build-scripts\run-dev.ps1`

## Notes

- Phase 0 targets Windows only.
- The current slice keeps engine integration behind adapter seams while the portable gameplay loop remains buildable and testable locally.
- GitHub Actions uses the same scripts as local development to reduce drift.
