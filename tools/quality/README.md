# Quality Tooling

Phase 6 quality tooling turns the portable shell into something we can gate like a release-quality beta.

- `peter_quality.py check-budgets` reads telemetry and writes reproducible budget reports.
- `peter_quality.py run-soak` runs repeated shell scenarios and writes a soak summary.
- `peter_quality.py verify-saves` scans profile saves and creator content for health issues.
- `peter_quality.py verify-profile` and `repair-profile --dry-run` support focused save checks.
- `peter_quality.py diff-save-revisions` exports a readable save-domain diff.
- `peter_quality.py export-qa-matrix` writes a generated QA summary under `Saved/Generated/quality/`.
- `peter_quality.py summarize-playtest` turns the playtest findings log into a compact summary.
- `peter_quality.py gate-rc` combines docs, budget, and save-health checks into one release gate.
