# ADR 0003: Portable Data Format Strategy

## Context

Gameplay rules, items, missions, tutorial steps, and creator-facing content must remain
portable across engines and easy to validate.

## Decision

- Store Phase 0 gameplay-authored data as JSON.
- Define each domain with JSON Schema.
- Require stable string IDs in dotted lowercase form.
- Keep shipped content separate from future user-edited content roots.

## Consequences

- Designers and tools can reason about a single portable format.
- Validation can run locally, in CI, and eventually in-editor.
- JSON verbosity is accepted in exchange for clarity and tooling compatibility.

## Open Questions

- Should higher-level authoring eventually use YAML or spreadsheet export before
  compiling down to JSON?
- Which schema draft should remain the long-term baseline once in-editor validation
  lands?
