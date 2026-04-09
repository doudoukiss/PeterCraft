# Onboarding Checklist

## Expected result

A new engineer can clone the repo, bootstrap dependencies, build the shell, run
validation, execute automated tests, and launch the empty runtime without undocumented
steps.

## Checklist

- Read `README.md`
- Read `docs/setup/WINDOWS_SETUP.md`
- Read `docs/setup/RUNTIME_MODES.md`
- Read `docs/CODING_STANDARDS.md`
- Run `bootstrap.ps1`
- Run `build-headless.ps1`
- Run `validate.ps1`
- Run `test.ps1`
- Run `run-headless.ps1`
- Run `build-playable.ps1`
- Run `run-playable.ps1`
- Confirm the headless shell creates a local profile stub and writes JSONL telemetry
- Confirm the playable preflight exits cleanly with a Phase 7.1 handoff message

## Troubleshooting

- If Visual Studio is installed but not on `PATH`, the build scripts locate it via `vswhere`.
- If `.venv` is missing, rerun `bootstrap.ps1`.
- If a new data file fails validation, confirm it matches the right schema in `game/data/schemas`.
