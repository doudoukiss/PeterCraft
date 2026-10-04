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

## GitHub automation policy

GitHub Actions is intentionally disabled for this owner-managed repository.
Do not re-enable Actions, add new Actions automation, or dispatch retained
workflows as routine setup or maintenance. Existing workflow YAML, badges and
upstream CI descriptions are references, not current validation results here.

Keep Dependabot automatic security updates disabled and do not add active
Dependabot version-update configuration. Preserve vulnerability alerts,
dependency graphs and secret scanning; review dependency changes manually.

Run applicable checks locally using this repository's existing documented tools
and lockfiles. Report actual results, failures and unavailable platform or release
gates. Local checks do not establish scientific validity, custody or production
acceptance. Pushes and tags do not themselves validate or authorize publication;
follow the owner's existing release/deployment procedure and authorization.

Deleted Actions artifacts are unavailable as release or recovery inputs. Obtain
fresh owner-approved evidence where needed, and preserve historical reports as
historical. This policy introduces no shared runner, sibling dependency or
replacement CI service.
