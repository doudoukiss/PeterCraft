# ADR 0005: Child-Safe Scripting Model

## Context

PeterCraft must eventually expose maker tools without turning the runtime into an
unsafe or opaque scripting host.

## Decision

- Keep child-facing editing behind curated APIs and data contracts.
- Disallow direct engine API exposure to creator flows.
- Require validation before activation for any user-authored logic or scripts.
- Keep creator code paths isolated from core runtime code and save domains.

## Consequences

- Workshop features can grow without risking unrestricted runtime access.
- Safety and explainability remain first-class requirements instead of later patches.
- Additional adapter and validation work is required before code mode ships.

## Open Questions

- Which future scripting runtime best balances sandboxing, performance, and teaching
  value?
- How much of the creator stack should run in-process versus in an isolated sandbox?
