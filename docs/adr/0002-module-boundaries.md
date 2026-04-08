# ADR 0002: Module Boundaries

## Context

PeterCraft needs strong subsystem boundaries so gameplay, workshop tooling, AI, and
platform concerns can evolve without tangling together.

## Decision

- Organize runtime code into named modules that mirror the architecture plan:
  `PeterCore`, `PeterTraversal`, `PeterCombat`, `PeterAI`, `PeterInventory`,
  `PeterProgression`, `PeterWorkshop`, `PeterWorld`, `PeterUI`, `PeterTelemetry`,
  `PeterDebug`, `PeterValidation`, and `PeterTools`.
- Keep `PeterCore` foundational and broadly reusable.
- Keep engine APIs outside the modules except through adapter interfaces.
- Require each module to document dependencies and public surface in its README.

## Consequences

- Dependency review becomes explicit instead of accidental.
- New engineers can map features to ownership more quickly.
- Cross-cutting features still need thoughtful coordination to avoid `PeterCore`
  becoming a dumping ground.

## Open Questions

- Which modules will eventually merge or split once real production load appears?
- Should some tool-only code move entirely out of the runtime tree in later phases?
