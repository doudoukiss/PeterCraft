# Repository Protection

Phase 0 expects the `main` branch to use GitHub branch protection with these settings:

- require pull requests before merging
- require the `validate_build_test` workflow to pass
- require branches to be up to date before merging
- block force pushes and branch deletion

These settings are applied in GitHub repository settings rather than in source control,
but this document is the required baseline.
