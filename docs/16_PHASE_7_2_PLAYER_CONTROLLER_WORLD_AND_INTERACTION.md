# 16. Phase 7.2 — Player Controller, World, and Interaction

## Objective

Create the first **moment-to-moment playable PeterCraft experience**:

- the player moves in a real world
- the camera feels good
- interaction is understandable
- one home base and one raid zone are physically navigable
- one extraction flow is embodied instead of simulated

This phase is where the project becomes truly playable.

## Principle

The primary quality bar is **feel**.

No one will care that the save schema is elegant if the character feels heavy, slippery, delayed, or awkward.

---

## Step 1 — Lock the player viewpoint and control fantasy

### Recommended viewpoint
Use **third-person over-the-shoulder** as the default.

### Why
- preserves readability for younger players
- makes the companion relationship more visible
- helps blocky animation and silhouette readability
- supports gentle action instead of twitch lethality
- matches the existing camera rig assumptions already visible in the repo

### Required actions
- move
- look
- sprint
- jump
- crouch
- interact
- primary action
- secondary action
- call companion / contextual prompt
- open inventory
- open explain panel
- pause

### Tasks
1. Formalize the control map in data.
2. Keep remapping support in scope.
3. Add controller parity early, not late.
4. Make comfort options part of the first implementation.

### Acceptance
A new player can move confidently within 30 seconds.

---

## Step 2 — Implement traversal feel to a premium standard

### Required movement behaviors
- responsive acceleration and deceleration
- jump buffering
- ledge forgiveness
- slope stability
- consistent sprint activation
- crouch readability
- no camera-motion nausea spikes

### Tasks
1. Convert `PeterTraversal` from profile description into real runtime movement behavior.
2. Keep tunable values in data/config.
3. Add a traversal debug panel:
   - speed
   - grounded state
   - jump buffer usage
   - camera mode
   - collision state
4. Capture input-to-motion latency metrics.

### Acceptance
Movement is reliable enough that playtesters comment on the game instead of on the controls.

---

## Step 3 — Build the home base as a real playable teaching space

### Home-base goals
The home base must be more than a menu room. It must teach the game.

### Required home-base stations
- mission board
- workshop bench
- companion terminal
- inventory/stash station
- results/mentor review nook

### Tasks
1. Build a small blockout home base with clear landmarks.
2. Place stations along a readable route.
3. Use environmental guidance:
   - shape language
   - lighting
   - signage
   - floor color/material contrast
4. Ensure every station is interactable through the real interaction system.

### Design rule
The home base should quietly teach:
- where missions start
- where gear is managed
- where companion behavior is understood
- where player-made changes happen

### Acceptance
A first-time player can find the mission board and workshop without external help.

---

## Step 4 — Build one real raid zone with world-class blockout discipline

### Scope
Build **one raid zone only** for this phase.

Use the existing authored mission/room/encounter data as a source of truth where practical, but simplify aggressively if the data set is too broad for the first in-engine slice.

### Recommended zone goals
- 8–10 minute target run length
- 5–7 rooms or beats
- 2 landmark spaces
- 1 optional loot detour
- 1 safe reset pocket
- 1 extraction endpoint

### Zone design rules
- every room must have a tactical identity
- no room exists only as filler
- landmarks must be readable in blockout
- verticality must support gameplay, not confusion
- pathing must be companion-safe and enemy-readable

### Tasks
1. Build a blockout room kit.
2. Create room metrics for:
   - traversal time
   - cover density
   - landmark quality
   - companion path safety
3. Reuse the repo’s content-review discipline.
4. Add in-world debug markers for room IDs and nav links in development builds.

### Acceptance
The raid zone is understandable as a place, not just as a sequence of boxes.

---

## Step 5 — Implement the interaction system

### Required interactions
- open container
- pick up item
- activate mission object
- use extraction
- operate station
- inspect companion explain terminal
- perform simple revive/help interaction if needed

### Interaction rules
- prompts must be consistent
- ranges must feel fair
- facing requirements must be readable
- prompts must not spam
- controller and keyboard prompts must both work

### Tasks
1. Define interactable categories in data.
2. Add an interaction trace/query system.
3. Create an interaction priority resolver for overlapping prompts.
4. Instrument failed interactions and abandoned prompts.
5. Provide accessibility-safe prompt presentation.

### Acceptance
A child understands why an object can or cannot be interacted with.

---

## Step 6 — Bring extraction into the physical world

### Current gap
Extraction today is largely resolved in scenario logic.

### Required future
The player must physically reach an extraction point, understand the countdown, survive pressure, and leave with emotional clarity.

### Tasks
1. Build a physical extraction zone.
2. Add entering, countdown, interruption, and completion states.
3. Use audio, HUD, and world feedback for the countdown.
4. Allow reduced-time-pressure accessibility settings to flow through the real countdown.
5. Keep success/failure reasons explicit.

### Acceptance
Extraction creates tension, but not confusion.

---

## Step 7 — Replace shell-only transitions with a felt gameplay loop

### Target loop
- home base
- mission select
- load raid
- play raid
- results
- return home

### Tasks
1. Build transition states with clear ownership.
2. Avoid modal confusion between gameplay and menus.
3. Ensure save writes happen at safe transition boundaries.
4. Add loading/transition telemetry.
5. Maintain narrative continuity through UI and audio cues.

### Acceptance
The loop feels like one coherent experience, not a stack of disconnected prototypes.

---

## Step 8 — Protect workshop integration without over-scoping it

### For this phase
Workshop integration should be minimal but real.

### Required proof
A player can change **one safe setting** or behavior chip in home base, then notice the effect in the next raid.

### Examples
- companion follow distance
- loot ping priority
- caution bias
- extraction preference bias

### Rule
Do not build the full creator dream here.  
Build one believable play-to-make bridge.

---

## Debug and telemetry requirements

Codex must add development visibility for:

- player position and velocity
- room ID
- current objective
- active interaction target
- current extraction state
- current input scheme
- camera mode
- traversal metrics
- scene transition timings

These are required, not optional.

---

## Required repository changes

Codex should expect to update or add:

- `game/code/gems/PeterTraversal/*`
- `game/code/gems/PeterWorld/*`
- `game/code/gems/PeterUI/*`
- `game/code/app/*`
- `game/assets/blockout/*`
- `game/ui/*`
- `game/audio/*`
- playable-scene/world assets
- new room metrics and review records

---

## Definition of done

This phase is done only when:

- the player can move, look, jump, sprint, crouch, and interact in a real world
- one home base is physically navigable
- one raid zone is physically navigable
- extraction exists as a world-space action
- one workshop change affects the next run
- transitions between spaces are coherent
- traversal, interaction, and extraction all feel good enough for first external playtests

## Failure modes to avoid

Do not:

- build a large map before controls feel good
- use placeholder layout with no landmark logic
- hide unclear interaction logic behind prompts
- treat extraction as only a UI timer
- overbuild creator systems before the embodied loop works
