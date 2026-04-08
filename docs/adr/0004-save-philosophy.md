# ADR 0004: Save Philosophy

## Context

PeterCraft is child-facing, offline-first, and must survive bad data or creator edits
without destroying progress.

## Decision

- Use local profile directories with explicit version metadata.
- Always create recoverable directories before writing save content.
- Separate shipped content, player progress, and future user-authored content.
- Keep save migration explicit and version-gated instead of implicit.

## Consequences

- Save corruption and migration risk are reduced early.
- Profile creation is testable before full gameplay save data exists.
- Additional work is required later to maintain migration tooling and backup rotation.

## Open Questions

- What backup retention policy should v1 use for autosaves and creator content?
- Which domains will be split into separate save files versus bundled snapshots?
