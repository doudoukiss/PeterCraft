# Release Candidate Policy

## Content lock

- New features stop unless they directly fix a P0/P1 issue.
- Content changes after lock require a linked issue and reviewer sign-off.

## Branch policy

- Stabilization happens on `release/x.y`.
- Main stays open for post-RC work unless an exception is approved.

## Must-pass gates

- validation
- build
- tests
- budget checks
- save-health checks
- smoke run
- RC gate report

## Exception process

- Late changes must document risk, rollback plan, and why the fix cannot wait.
