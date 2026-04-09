# ADR 0006: Runtime Modes and Backend Composition

## Context

PeterCraft is entering Phase 7, where the project must move from a portable headless shell
to a real playable runtime without breaking tests, validation, save safety, or deterministic
tooling.

Before engine integration starts, the repository needs an explicit way to separate:

- the current headless/test runtime
- the future playable/engine-backed runtime

## Decision

- Add explicit runtime modes: `headless` and `playable`.
- Keep `headless` as the default runtime for tests, validation, deterministic scenarios, and CI smoke.
- Keep `NullPlatformServices` as the first-class headless backend.
- Select backends through a runtime descriptor and platform-services factory instead of spreading backend conditionals across game logic.
- Add the `playable` path in Phase 7.0 as a clean preflight-only stub that exits with a structured “backend unavailable until Phase 7.1” result.
- Keep the actual engine-backed backend out of Phase 7.0.

## Consequences

- Contributors can tell the difference between the current stable runtime and the upcoming playable path.
- CI and local scripts can describe headless vs playable clearly instead of using vague “dev” naming.
- Phase 7.1 can add the O3DE adapter stack behind the same app/runtime boundary instead of rewriting the shell.
- Transitional feature flags and docs are required during the migration.

## Open Questions

- How much of the future playable backend should live in `engine-adapters/` versus app composition glue?
- Which playable smoke checks should move into CI first once the real O3DE path exists?
- When should the temporary Phase 7 migration flags be removed?
