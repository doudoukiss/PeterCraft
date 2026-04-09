# 20. Phase 7 Execution Backlog

## Objective

Provide an ordered implementation backlog for Codex so the playable upgrade lands in the correct sequence.

This backlog is intentionally narrow. It is designed to produce **one world-class playable slice**, not a broad proto-live-service game.

## Execution order

Follow the backlog in order.  
Do not pull content polish ahead of engine and feel foundations.

---

# Epic P7-00 — Build and Runtime Hardening

## P7-001 Fix strict-warning build blockers
- **Goal:** make the codebase build cleanly under strict warnings
- **Touches:** `PeterCombat`, any affected modules
- **Depends on:** none
- **Done when:** CI/local strict builds are green

## P7-002 Remove unconditional platform-specific links
- **Goal:** guard OS-specific dependencies correctly
- **Touches:** `PeterTelemetry` CMake and related modules
- **Depends on:** P7-001
- **Done when:** Windows and Linux compile smoke both pass

## P7-003 Formalize runtime modes
- **Goal:** distinguish headless vs playable runtime cleanly
- **Touches:** `game/code/app`, docs, build scripts
- **Depends on:** P7-001
- **Done when:** runtime selection is explicit and documented

## P7-004 Add Phase 7 quality profile and metrics
- **Goal:** define the playable-slice budgets before implementation
- **Touches:** `PeterCore`, `PeterTelemetry`, quality tools
- **Depends on:** P7-001
- **Done when:** budgets are reportable in tooling

## P7-005 Update CI and local workflows
- **Goal:** support both headless and playable build paths
- **Touches:** `.github/workflows`, `tools/build-scripts`, setup docs
- **Depends on:** P7-002, P7-003
- **Done when:** contributors can build both modes repeatably

---

# Epic P7-10 — O3DE Playable Baseline

## P7-101 Create O3DE project scaffold
- **Goal:** register PeterCraft as a real O3DE-backed project
- **Touches:** new O3DE project/runtime directories, docs
- **Depends on:** P7-003, P7-005
- **Done when:** engine project opens and builds

## P7-102 Implement O3DE input adapter
- **Goal:** feed real input into the portable core
- **Touches:** `engine-adapters/o3de`, adapter interfaces
- **Depends on:** P7-101
- **Done when:** keyboard/controller input is sampled in runtime

## P7-103 Implement O3DE camera adapter
- **Goal:** provide third-person OTS camera control
- **Touches:** `engine-adapters/o3de`, traversal integration
- **Depends on:** P7-101
- **Done when:** camera rig can be queried and applied at runtime

## P7-104 Implement O3DE save, audio, navigation, and UI adapters
- **Goal:** replace null services in the playable runtime
- **Touches:** `engine-adapters/o3de`
- **Depends on:** P7-101
- **Done when:** all adapter categories exist in playable mode

## P7-105 Boot the first engine-backed room
- **Goal:** prove end-to-end playable infrastructure in one tiny room
- **Touches:** app shell, scene loading, minimal assets
- **Depends on:** P7-102, P7-103, P7-104
- **Done when:** the player can move and interact in one room

---

# Epic P7-20 — Traversal and World Embodiment

## P7-201 Implement real traversal controller
- **Goal:** make movement feel good
- **Touches:** `PeterTraversal`, adapter integration
- **Depends on:** P7-105
- **Done when:** movement metrics and feel reviews pass

## P7-202 Build blockout home base
- **Goal:** create the first navigable safe space
- **Touches:** world assets, `PeterWorld`, UI prompts
- **Depends on:** P7-105
- **Done when:** player can navigate and use home stations

## P7-203 Build blockout raid zone
- **Goal:** create the first navigable dangerous space
- **Touches:** world assets, mission data, room reviews
- **Depends on:** P7-105
- **Done when:** a full route from spawn to extraction exists

## P7-204 Implement interaction system
- **Goal:** support prompts, object use, loot, and station access
- **Touches:** `PeterTraversal`, `PeterWorld`, `PeterUI`
- **Depends on:** P7-201, P7-202, P7-203
- **Done when:** interaction feels consistent and readable

## P7-205 Implement real scene transitions
- **Goal:** replace fake scene changes with actual loading
- **Touches:** `PeterWorld`, app shell, O3DE scene management
- **Depends on:** P7-202, P7-203
- **Done when:** home -> raid -> results -> home works in runtime

## P7-206 Implement physical extraction zone
- **Goal:** embody the extraction mechanic
- **Touches:** `PeterWorld`, `PeterUI`, audio cues, mission state
- **Depends on:** P7-203, P7-204
- **Done when:** extraction can start, interrupt, and complete in-world

---

# Epic P7-30 — Combat and AI Runtime

## P7-301 Refactor combat for real-time execution
- **Goal:** connect portable combat logic to runtime actions
- **Touches:** `PeterCombat`, debug/telemetry hooks
- **Depends on:** P7-201
- **Done when:** player attack/hit/damage loop works in runtime

## P7-302 Implement enemy archetype A
- **Goal:** basic patrol/engage threat
- **Touches:** `PeterAI`, `PeterCombat`, world encounters
- **Depends on:** P7-301, P7-203
- **Done when:** one enemy patrols, notices, and attacks

## P7-303 Implement enemy archetype B
- **Goal:** second distinct threat pattern
- **Touches:** `PeterAI`, `PeterCombat`, encounter data
- **Depends on:** P7-302
- **Done when:** testers can clearly tell the difference

## P7-304 Implement companion runtime behavior
- **Goal:** real-time helpful teammate behavior
- **Touches:** `PeterAI`, navigation, HUD/audio callouts
- **Depends on:** P7-301, P7-203
- **Done when:** companion follows, helps, and stays explainable

## P7-305 Port explainability into playable runtime
- **Goal:** keep AI visible and understandable
- **Touches:** `PeterUI`, `PeterAI`, debug overlay
- **Depends on:** P7-304
- **Done when:** a player can inspect a companion decision in runtime

## P7-306 Implement mission runtime state machine
- **Goal:** make objective flow real, observable, and robust
- **Touches:** `PeterWorld`, mission data, telemetry
- **Depends on:** P7-203, P7-206, P7-302, P7-304
- **Done when:** mission start, progress, fail, and success all work

## P7-307 Preserve failure/recovery persistence loop
- **Goal:** keep inventory loss, recovery, and results trustworthy
- **Touches:** `PeterInventory`, `PeterProgression`, save domains
- **Depends on:** P7-306
- **Done when:** success and failure both persist safely

---

# Epic P7-40 — HUD, Audio, and Educational Bridge

## P7-401 Build the player-facing HUD
- **Goal:** clear objective, health, companion, and extraction visibility
- **Touches:** `PeterUI`, UI assets
- **Depends on:** P7-206, P7-306
- **Done when:** core state is understandable without debug tools

## P7-402 Add audio cue pass
- **Goal:** support key state changes with sound
- **Touches:** audio assets, audio adapter, content data
- **Depends on:** P7-206, P7-302, P7-304
- **Done when:** extraction, threat, loot, and interaction are sonically legible

## P7-403 Add blockout art kit and lighting pass
- **Goal:** make spaces intentional and readable
- **Touches:** `game/assets`, world scenes
- **Depends on:** P7-202, P7-203
- **Done when:** screenshots read clearly

## P7-404 Rebuild first-run onboarding for real-time play
- **Goal:** teach through action instead of shell panels
- **Touches:** tutorial data, HUD copy, mission beats
- **Depends on:** P7-401, P7-402, P7-403
- **Done when:** new players can reach extraction naturally

## P7-405 Implement one workshop-to-next-run proof
- **Goal:** preserve PeterCraft’s learning promise in the playable slice
- **Touches:** `PeterWorkshop`, `PeterUI`, companion config flow
- **Depends on:** P7-304, P7-401
- **Done when:** one safe edit visibly changes the next run

---

# Epic P7-50 — Validation, Playtests, and Release Readiness

## P7-501 Add playable smoke tests
- **Goal:** verify boot, load, interact, and return loop
- **Touches:** tests, runtime automation
- **Depends on:** P7-205, P7-306
- **Done when:** automated playable smoke exists

## P7-502 Add performance capture and reporting
- **Goal:** measure real play budgets
- **Touches:** telemetry, quality tools, nightly jobs
- **Depends on:** P7-401
- **Done when:** reports exist for frame, boot, memory, and transition

## P7-503 Run internal and external playtests
- **Goal:** validate feel, clarity, and trust
- **Touches:** playtest docs, telemetry exports, issue tracker
- **Depends on:** P7-404, P7-405
- **Done when:** action items are recorded and triaged

## P7-504 Fix high-severity issues from playtests
- **Goal:** reach a true world-class playable bar
- **Touches:** whichever systems fail review
- **Depends on:** P7-503
- **Done when:** no stop-ship items remain

## P7-505 Approve Phase 7 exit review
- **Goal:** close the milestone only on evidence
- **Touches:** docs, quality reports, release checklist
- **Depends on:** P7-501, P7-502, P7-504
- **Done when:** leads sign off that PeterCraft is genuinely playable

---

## Must / Should / Could

### Must
- clean build
- O3DE playable runtime
- one real home base
- one real raid zone
- one companion
- two enemy archetypes
- extraction
- persistence
- HUD
- audio basics
- one workshop effect
- playtests
- quality gates

### Should
- controller parity
- replay/event export for bug repro
- richer companion callouts
- more polished blockout lighting
- improved results screen

### Could
- optional side room
- one extra support gadget
- one extra explain visualization
- richer mentor summary output

---

## Sequencing rule

Do not start work from later epics while earlier epics are structurally incomplete.

Specifically:

- no art pass before movement feels good
- no onboarding pass before the playable loop works
- no content breadth before one mission is excellent
- no “juicy polish” before save safety and AI trust are proven

## Final success statement

When this backlog is complete, PeterCraft will no longer be merely well-designed on paper.

It will be a **real, replayable, educational extraction-adventure slice** with the foundations required for world-class expansion.
