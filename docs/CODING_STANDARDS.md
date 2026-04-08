# Coding Standards

## C++ style

- Standard: C++20
- Indentation: 4 spaces
- Braces: opening brace on the same line
- Naming:
  - types and namespaces use `PascalCase`
  - functions use `PascalCase`
  - local variables use `camelCase`
  - constants use `kPascalCase`
  - file names use `PascalCase` for C++ source owned by a module
- Headers should include only what they use.
- Prefer `std::string_view` for read-only string parameters.

## Repository conventions

- One public header per module should describe the module entry point.
- Module READMEs must list owner, responsibilities, dependencies, and public surface.
- Portable gameplay data is JSON and must be schema validated.
- Stable IDs use dotted lowercase tokens such as `item.salvage.scrap_metal`.
- Child-facing creator logic must not directly depend on engine APIs.

## Testing rules

- Keep at least one unit, integration, deterministic scenario, and smoke test passing.
- Prefer deterministic tests over time-based or machine-state-dependent tests.
- New shared utilities require unit coverage or an explicit rationale in the pull request.

## Machine-checkable rules

- No tabs in tracked text files.
- No trailing whitespace outside Markdown.
- Every JSON data file must validate against a schema.
- Every module directory must contain `README.md`, `CMakeLists.txt`, `include/`, `src/`, and `tests/`.
