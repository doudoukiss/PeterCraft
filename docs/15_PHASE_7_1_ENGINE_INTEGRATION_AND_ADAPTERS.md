# 15. Phase 7.1 — Engine Integration and Adapter Implementation

## Objective

Create the first **real engine-backed playable runtime** for PeterCraft using **O3DE** as the primary path, while preserving the repository’s engine-agnostic core.

This phase is about making PeterCraft boot into a real playable world.

## Strategic rule

O3DE remains the primary choice.

Unreal is **not** the default pivot.  
An Unreal fallback is allowed only if the formal engine gates fail after a focused O3DE spike.

## Why this is the right engine move

The current codebase is already organized around portable systems and adapters. That means the correct next step is not to rewrite PeterCraft around engine-native gameplay scripts. The correct next step is to give the existing architecture a real engine host.

## Core architecture for this phase

```text
PeterCraft Playable Runtime
├─ O3DE Project Layer
├─ Engine Adapter Implementations
│  ├─ Input Adapter
│  ├─ Camera Adapter
│  ├─ Save Adapter
│  ├─ Navigation Adapter
│  ├─ Audio Adapter
│  └─ UI Adapter
├─ Portable Game Core
│  ├─ PeterCore
│  ├─ PeterTraversal
│  ├─ PeterCombat
│  ├─ PeterAI
│  ├─ PeterInventory
│  ├─ PeterProgression
│  ├─ PeterWorkshop
│  ├─ PeterWorld
│  └─ PeterUI
└─ Headless Test Runtime
   └─ NullPlatformServices
```

The engine hosts the game.  
The portable core still defines the game.

---

## Step 1 — Create the O3DE playable project skeleton

### Tasks
1. Create a dedicated O3DE project layer for PeterCraft.
2. Pin the engine version in repo docs and bootstrap scripts.
3. Document engine installation, project registration, and build steps.
4. Separate engine project files from pure gameplay-core files.
5. Add an ADR describing why O3DE is now an active runtime target rather than a paper baseline.

### Recommended structure
Add a new repo area such as:

- `engine-adapters/o3de/`
- `game/o3de/` or `runtime/o3de/`
- `docs/setup/O3DE_SETUP.md`

### Non-negotiable rule
Do not move portable gameplay data into O3DE-only asset formats unless there is a concrete runtime reason.

---

## Step 2 — Implement real adapter backends

The current adapter contracts in `engine-adapters/include/PeterAdapters/PlatformServices.h` are the correct seam. Use them.

### Input adapter
Must provide:
- mouse/keyboard support
- controller support
- remappable action model
- active scheme reporting
- low-latency sampling path

### Camera adapter
Must provide:
- third-person over-the-shoulder rig
- runtime rig changes
- motion-comfort variants
- wall/collision safety behavior
- non-fighting camera follow

### Save adapter
Must provide:
- profile root resolution
- safe directory creation
- compatibility with existing save-domain architecture
- clear separation between shipped content and player content

### Navigation adapter
Must provide:
- path resolution hooks for enemies and companion
- navmesh-backed queries
- debug visibility for routes and failures

### Audio adapter
Must provide:
- UI cues
- world cues
- prioritized feedback cues
- concurrency management
- debug logging of cue playback in development builds

### UI adapter
Must provide:
- actual menu and HUD presentation
- prompt presentation
- panel presentation
- creator panel presentation
- explain/replay presentation
- accessibility settings application

### Rule
Do not let the engine implementation leak raw engine types throughout the portable core.

---

## Step 3 — Preserve the null backend as a first-class test backend

### Tasks
1. Keep `CreateNullPlatformServices`.
2. Ensure all non-rendering tests still run against null services.
3. Add tests proving that portable gameplay logic does not require the O3DE backend.
4. Keep deterministic harnesses independent of engine-frame timing.

### Why
This is the safeguard that keeps PeterCraft maintainable.

---

## Step 4 — Create the real boot flow

### Today
The app boots, emits events, and prints output.

### Required future
The playable runtime must:
1. initialize the engine
2. register the PeterCraft project/runtime services
3. create platform services from O3DE
4. mount content roots
5. load a playable startup world
6. hand execution to the runtime shell

### Tasks
1. Replace or extend `PeterCraftApp` so it supports a playable boot path.
2. Keep the headless path available.
3. Introduce a clean startup selection strategy such as:
   - `--runtime headless`
   - `--runtime playable`
4. Ensure boot telemetry still fires in both paths.

---

## Step 5 — Replace fake scene switching with real world transitions

### Current issue
`game/code/gems/PeterWorld/src/SceneShell.cpp` emits scene events but does not load real scenes.

### Required change
Introduce a true scene/level transition service.

### Tasks
1. Define real home-base, raid, and results world assets.
2. Keep the `SceneShell` concept, but turn it into a real transition coordinator.
3. Separate:
   - logical scene intent
   - engine scene/level loading
4. Ensure loading and transition timings still feed telemetry.
5. Add loading screens or transition cards only if required for usability.

### Acceptance
When the player selects a mission, the game actually loads the raid scene.

---

## Step 6 — Build the O3DE “one-room proof” before full slice integration

### Purpose
Reduce risk.

### Required proof
Before a full playable slice exists, Codex must prove all of the following in one tiny room:

- the player can move
- the camera works
- the UI can show a prompt
- the player can interact with an object
- the companion can path to the player
- one enemy can move and attack
- one audio cue plays
- one save write succeeds

If this room fails, do not scale to a map.

---

## Step 7 — Add engine decision gates and fallback measurement

### O3DE must be measured now, not assumed forever

### Required measurements
- time to first playable room
- time to add one interactable
- time to add one enemy archetype
- time to author one blockout room
- time to debug one AI route failure
- build reproducibility
- average iteration time for a gameplay parameter change

### Unreal fallback condition
Only trigger an Unreal reevaluation if O3DE materially fails on iteration speed, authoring throughput, performance, or operational stability after this spike.

### Rule
Do not migrate because Unreal is popular.  
Migrate only if PeterCraft’s concrete needs are not being met.

---

## Step 8 — Update docs and contributor workflows

### Required outputs
- `docs/setup/O3DE_SETUP.md`
- new ADR for playable runtime baseline
- updated repo README with runtime modes
- build scripts for playable boot
- debug instructions for adapter failures
- troubleshooting guide for asset registration and world loading

---

## Required repository changes

Codex should expect to add or update:

- `engine-adapters/o3de/*`
- `engine-adapters/include/PeterAdapters/PlatformServices.h`
- `engine-adapters/src/NullPlatformServices.cpp`
- `game/code/app/*`
- `game/code/gems/PeterWorld/src/SceneShell.cpp`
- `docs/setup/*`
- `tools/build-scripts/*`
- `.github/workflows/*`

---

## Definition of done

This phase is done only when:

- PeterCraft boots into a real O3DE-backed runtime
- the null backend still exists and still powers headless tests
- scene transitions are real
- adapters are cleanly separated
- one tiny room proves end-to-end playability
- engine iteration metrics are documented
- O3DE remains the chosen baseline unless the documented gates fail

## Failure modes to avoid

Do not:

- rewrite core rules as engine-only scripts
- bury engine calls directly into every gameplay module
- delete headless mode
- overbuild the project skeleton before a tiny room proves the concept
- pivot to Unreal without measured evidence
