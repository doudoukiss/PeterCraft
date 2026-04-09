# 13. Phase 7 — Playable Real-Time Vertical Slice

## Objective

Convert PeterCraft from a high-quality simulation shell into a **real-time playable game** without sacrificing the project’s strongest architectural properties:

- portable game rules
- deterministic core logic
- explainable AI
- safe saves
- child-friendly clarity
- creator-tool integrity

Phase 7 is the first milestone where PeterCraft must feel like a game instead of a systems demo.

## The core decision

The next huge upgrade is **not**:

- more authored content
- more workshop lesson types
- more review templates
- more data schemas
- more scenario harnesses
- more menu states

The next huge upgrade **is**:

- a real engine-backed runtime
- real player control
- real camera and traversal
- real raid-space navigation
- real combat readability
- real AI companion behavior in-world
- real extraction tension
- real save-backed outcomes

## Phase 7 north-star experience

A child launches PeterCraft, moves through a welcoming home base, chooses a mission, enters a dangerous blockout zone, explores with a companion, defeats threats, collects loot, reaches extraction, and returns home with a story they can explain.

The child should say:

- “I know what I was trying to do.”
- “I know why I failed.”
- “My companion helped me.”
- “I changed something and it mattered.”
- “I want another run.”

If the player cannot honestly say those five things, the slice is not complete.

## World-class design pillars for this phase

### 1. Responsiveness over feature count
One excellent movement controller is worth more than five incomplete systems.

### 2. Readability over realism
PeterCraft should be stylized, legible, and emotionally safe for an 11-year-old.

### 3. Explainability over mystery
Companion behavior must remain understandable even after it becomes real-time.

### 4. Playability over paper breadth
One exceptional mission beats three fake ones.

### 5. Portability over engine lock-in
The core game must still live in portable code and data.

## The exact Phase 7 slice

### Required
- over-the-shoulder third-person player controller
- one functional home-base scene
- one functional raid scene
- one mission start flow
- one mission objective set
- one extraction point with countdown
- one AI companion with follow, defend, loot, revive, and extract priorities
- two enemy archetypes
- one lootable container family
- one carry/loadout model
- one results + persistence loop
- one workshop effect that changes the next run
- basic HUD
- basic audio cues
- debug overlay and telemetry
- accessibility baseline
- automated build + smoke tests

### Explicitly out of scope
- MMO or online multiplayer
- procedural world scale
- unrestricted child modding
- final art
- multiple raid zones
- broad weapon taxonomy
- advanced cinematic systems
- voice acting
- marketplace-scale content throughput

## Why this phase matters

The current repository has already done unusually disciplined early work:

- module boundaries are clear
- content is portable
- saves are structured
- AI is explainable
- validation is integrated
- debug and telemetry exist

That means PeterCraft has a rare opportunity: it can become playable **without becoming messy**.

Phase 7 must protect that advantage.

## Current repository reality

Codex must design this phase against the codebase that exists today.

### Current strengths
- `PeterCore`, `PeterAI`, `PeterInventory`, `PeterWorkshop`, `PeterWorld`, `PeterTelemetry`, and other gems already exist.
- headless logic and portable catalogs are in place
- mission, encounter, room, and tutorial data already exist
- creator content is versioned and stored safely
- quality and validation tooling already exist

### Current limits
- `NullPlatformServices` still owns runtime presentation
- `SceneShell` still reports scene changes instead of driving real world loading
- `SlicePresentation` still renders text output instead of actual HUD/UI widgets
- asset, UI, and audio directories are placeholders
- the main loop still resolves encounters as scenario logic rather than embodied gameplay
- real traversal, camera, real-time hit feedback, and in-world companion execution are absent

## The major architectural rule

**Do not throw away the current architecture.**

Instead:

- keep the portable core
- keep null adapters for tests
- keep deterministic simulation harnesses
- add a real engine integration layer
- let the engine host and present the already-defined game rules

That is the right world-class move.

## Phase 7 milestone sequence

### P7.0 — Stabilize the base
Make the repository green, cross-toolchain-aware, and ready for real engine integration.

### P7.1 — Engine-backed runtime
Implement the first real O3DE-backed adapter stack while preserving headless mode.

### P7.2 — Real-time traversal and interaction
Ship movement, camera, interactions, one home base, and one raid blockout.

### P7.3 — Real-time combat, AI, and mission loop
Bring enemies, companion support, objective state, loot, extraction, and failure/recovery into the world.

### P7.4 — HUD, audio, onboarding, and blockout art
Turn the playable slice into something understandable and emotionally satisfying.

### P7.5 — Quality proving
Run the slice through world-class engineering, design, and playtest gates.

## Phase 7 stop-ship conditions

Any one of the following means Phase 7 is not done:

- player movement feels stiff or unreliable
- camera comfort is poor
- enemy attacks are unreadable
- the companion behaves in surprising or untrustworthy ways
- extraction pressure feels arbitrary
- save/load can corrupt the playable state
- workshop changes can damage the profile
- the first run is confusing for a child
- the slice lacks debug visibility
- the slice only works in-editor and not as a real build

## Repo areas that Phase 7 must touch

### Must evolve
- `engine-adapters/`
- `game/code/app/`
- `game/code/gems/PeterTraversal/`
- `game/code/gems/PeterCombat/`
- `game/code/gems/PeterAI/`
- `game/code/gems/PeterWorld/`
- `game/code/gems/PeterUI/`
- `game/assets/`
- `game/audio/`
- `game/ui/`
- `tools/build-scripts/`
- `.github/workflows/`

### Must remain valuable
- `game/data/`
- `tests/`
- `tools/content-validation/`
- `tools/quality/`
- `docs/quality/`

## Required final demonstration for this phase

The slice demo should be short and disciplined:

1. start from a fresh profile
2. enter home base
3. open mission board
4. launch the raid
5. traverse a route with one vertical movement beat and one interact gate
6. fight one patrol enemy
7. watch the companion make an explainable helpful choice
8. loot one high-value object
9. survive a second threat or pressure beat
10. extract with countdown
11. return home with persistent rewards
12. apply one workshop change
13. rerun and observe the difference

If this flow feels coherent and premium, PeterCraft has crossed the line into “playable.”

## Deliverables

Codex must produce all of the following during Phase 7:

- green build and CI updates
- O3DE adapter baseline
- one playable home base
- one playable raid map
- one playable mission
- one functional HUD
- one audio feedback pass
- one companion explain panel in the real runtime
- one workshop-to-next-run proof
- new ADRs and updated docs
- new smoke and regression tests

## Acceptance criteria

Phase 7 is complete when:

- the project opens as a real game, not only as a console shell
- a player can physically move, fight, loot, extract, and return home
- the companion behaves in real time and remains explainable
- the current save architecture survives the playable loop
- workshop edits safely affect runtime behavior
- performance, QA, and playtest evidence meet the defined bar
- the build remains architecturally clean enough to scale later
