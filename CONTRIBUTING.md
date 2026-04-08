# Contributing

## Branch and commit hygiene

- Keep changes scoped to one purpose.
- Do not mix architecture, content, and unrelated cleanup in one commit.
- Use imperative commit titles such as `Add profile shell boot flow`.
- Include validation and test evidence in pull requests.
- Treat failing validation, missing docs, or missing telemetry as incomplete work.

## Coding standards

- C++ uses C++20 and the repository `.editorconfig`.
- Prefer clear, named types over clever templates.
- Keep engine-specific code inside `engine-adapters/` or dedicated adapter seams.
- Every new data file needs a schema or an existing schema reference.
- Every new module or public interface needs a short README update.

## Pull request expectations

- `bootstrap`, `build`, `validate`, and `test` must pass locally before review.
- Runtime changes should emit or update structured events when behavior changes.
- Child-facing flows must preserve preview, apply, undo, or reset semantics where relevant.
- If a decision changes architecture, add or update an ADR in `docs/adr/`.
