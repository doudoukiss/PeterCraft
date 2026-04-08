# ADR 0001: O3DE-First Engine Baseline

## Context

PeterCraft is planned as an O3DE-first project, but Phase 0 is focused on stable
contracts, tooling, and a bootable shell rather than full engine integration.

## Decision

- Treat O3DE as the approved primary engine path.
- Keep Phase 0 runtime code engine-agnostic and compile it with standard C++ so the
  foundation remains testable before full engine binding lands.
- Require all future engine integration to flow through `engine-adapters/`.
- Keep the exact engine package pin as an environment-level project baseline recorded by
  engineering once the feasibility spike output is checked into the repo.

## Consequences

- Phase 0 can move forward without waiting on a local engine checkout.
- Future O3DE work must honor the adapter seams instead of binding gameplay code
  directly to engine APIs.
- The repository gains a portable shell that can be validated in CI before content-heavy
  engine work arrives.

## Open Questions

- Which exact O3DE distribution and patch line will the feasibility spike ratify?
- Will the engine be vendored, submoduled, or referenced from a shared install?
