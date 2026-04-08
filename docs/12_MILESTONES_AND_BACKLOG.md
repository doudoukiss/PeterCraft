# 12. Milestones and Backlog

## Objective

Give Codex a practical sequence for executing PeterCraft in logical, testable milestones.

## Delivery philosophy

The goal is not to implement every idea at once. The goal is to reach a world-class result through layered proof:

1. prove the engine path
2. prove the core loop
3. prove the systems
4. prove the learning loop
5. prove scale and polish
6. ship only when the world-class bar is met

## Milestone sequence

### Milestone 0: Engine and feasibility spike
Goal:
- confirm O3DE viability for PeterCraft

Build:
- one tiny raid room
- one AI companion stub
- one enemy stub
- one extraction interaction
- one editable rule

Exit:
- decision gates reviewed
- O3DE locked or formal Unreal re-evaluation triggered

### Milestone 1: Foundation
Goal:
- create a stable project skeleton

Build:
- repo layout
- CI
- module scaffolding
- schemas
- validation
- test harness
- bootable shell

Exit:
- new developers can build and run cleanly

### Milestone 2: Vertical slice
Goal:
- prove the full loop

Build:
- home base
- one raid
- one extraction
- one companion
- one enemy family
- loot, stash, and one upgrade
- explainability panel
- one behavior edit

Exit:
- fresh players understand the loop and want another run

### Milestone 3: Core systems alpha
Goal:
- deepen systems enough to support content expansion

Build:
- combat pipeline
- item taxonomy
- mission templates
- progression scaffolding
- fair failure/recovery systems
- post-raid clarity

Exit:
- content designers can author multiple meaningful variants

### Milestone 4: AI and creator alpha
Goal:
- prove the educational differentiation

Build:
- companion stances
- behavior chips
- explainability UX
- tinker mode
- logic mode
- first code mode lesson

Exit:
- target-age playtesters can move from playing to modifying behavior

### Milestone 5: Content beta
Goal:
- scale to a true product

Build:
- additional raid zones
- additional mission templates
- content tools
- validation and review pipeline
- more workshop lessons
- expanded progression

Exit:
- the team can produce content predictably and safely

### Milestone 6: Polish beta
Goal:
- raise every discipline to premium quality

Build:
- UX polish
- audio pass
- performance pass
- accessibility pass
- save hardening
- QA matrix

Exit:
- most feedback is about preferences, not confusion or breakage

### Milestone 7: Release candidate
Goal:
- stabilize and ship

Build:
- bug fixes
- balance fixes
- onboarding tuning
- final content reviews
- final compatibility and migration checks

Exit:
- all ship criteria pass

## Priority backlog

### P0 — absolutely required
- responsive movement and camera
- one compelling extraction loop
- trustworthy local saves
- one useful companion
- explainability panel
- one safe behavior edit
- one workshop lesson
- validation for content and creator edits
- CI and deterministic tests

### P1 — strongly recommended for v1
- 3 raid zones
- 6–8 mission templates
- multiple companion stances
- tinker mode
- logic mode
- introductory code mode
- room authoring tools
- replay or comparison tools
- accessibility feature set
- release-quality audio pass

### P2 — later or optional
- advanced procedural variation
- richer narrative layer
- mentor dashboards
- exportable creator reports
- tablet/mobile adaptation
- post-launch content packs
- controlled community sharing

## Cross-functional workstreams

Codex should track work in parallel by stream:

### Stream A: Player loop
movement, interaction, raids, combat, extraction, rewards

### Stream B: AI
perception, decisions, action library, explainability

### Stream C: Workshop
creator UI, tutorials, validation, scripting safety

### Stream D: Content
rooms, encounters, missions, tutorials, progression

### Stream E: Platform quality
saves, performance, accessibility, QA, build pipeline

### Stream F: Tooling
schemas, validation, automation, debug overlays, authoring tools

## Suggested milestone review template

For every milestone, Codex should produce:
- what was built
- what is still fake or placeholder
- what was learned
- what failed
- what changed in the design
- risks for the next milestone
- playtest evidence
- performance and stability evidence

## Final note

PeterCraft only works if the game, the learning ladder, and the creation tools all rise together. Do not postpone the educational layer until the end. It is the differentiator, not a bonus feature.
