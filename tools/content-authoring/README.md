# Content Authoring

Phase 5 adds thin authoring helpers around the shipped content catalogs.

- `peter_content.py scaffold-room` creates a new room-variant draft.
- `peter_content.py scaffold-encounter` creates a new encounter-pattern draft.
- `peter_content.py scaffold-mission` creates a new mission-blueprint draft.
- `peter_content.py preview-mission-graph` renders a text preview for one mission.
- `peter_content.py preview-encounter` renders a text preview for one encounter.
- `peter_content.py capture-previews` writes preview artifacts under `Saved/Generated/content-previews/`.
- `peter_content.py export-room-metrics` writes room metrics under `Saved/Generated/room-metrics/`.
- `peter_content.py diff-content` writes a Markdown diff report under `Saved/Generated/content-diffs/`.
