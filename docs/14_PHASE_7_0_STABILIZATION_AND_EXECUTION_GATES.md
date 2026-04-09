# 14. Phase 7.0 — Stabilization and Execution Gates

## Objective

Create a clean, trustworthy foundation for the playable upgrade before any engine integration work begins.

The project must not attempt a real-time playability jump on top of a flaky build, platform-specific linkage assumptions, or an unmeasured performance target.

## Why this phase exists

The repository is already well-planned, but the review of the actual codebase exposed immediate execution blockers:

- strict-warning builds fail in at least one combat translation unit
- telemetry links `Psapi` unconditionally even though the code is conditionally compiled
- CI only proves a Windows-first path
- adapters are null-only today
- playable-content directories are still placeholders

World-class teams do not step over these issues. They remove them before scaling the project.

## Codex rule for this phase

No gameplay features.  
No content expansion.  
No visual polish.

This phase exists to guarantee that later work lands on a stable, measurable baseline.

---

## Step 1 — Make the repository green under strict warnings

### Tasks
1. Reproduce current build failures locally and in CI.
2. Fix all strict-warning failures without disabling warning quality.
3. Add initialization-safe defaults for all aggregate structs that are warning-prone.
4. Review any aggregate initialization in combat, AI, world, and save code.
5. Add tests for the fixed code paths if behavior could change.

### Immediate known target
- `game/code/gems/PeterCombat/src/EncounterSimulator.cpp`

### Standards
- no “quick suppression” unless the warning is demonstrably noisy and low-value
- prefer explicit initialization and safer constructors
- do not reduce warning strictness for the whole project to fix one file

### Acceptance
- Debug and Release builds succeed with `PETERCRAFT_ENABLE_STRICT_WARNINGS=ON`

---

## Step 2 — Remove accidental Windows-only linkage assumptions

### Tasks
1. Audit all libraries linked unconditionally in CMake.
2. Gate platform-specific dependencies with platform checks.
3. Provide a null or portable fallback path when OS-specific APIs are unavailable.
4. Keep behavior identical on Windows.
5. Add at least one non-Windows configure/build path to protect against silent regressions.

### Immediate known target
- `game/code/gems/PeterTelemetry/CMakeLists.txt`
- `game/code/gems/PeterTelemetry/src/QualityMetrics.cpp`

### Design rule
The playable build can still be PC-first, but the core codebase must not silently rot outside a single compiler/platform family.

### Acceptance
- Linux or Clang/GCC compile smoke passes for the portable core and tools
- Windows builds still pass
- no unconditional link to `Psapi` on non-Windows platforms

---

## Step 3 — Split “headless runtime” from “real playable runtime” explicitly

### Problem
The repository currently blurs two valid modes:

1. **headless/test runtime**
2. **playable engine-backed runtime**

Both are useful and should continue to exist.

### Tasks
1. Introduce explicit runtime mode naming in docs and code.
2. Preserve `NullPlatformServices` as the headless test/pipeline backend.
3. Define a future `O3DEPlatformServices` backend as the playable runtime backend.
4. Make the application shell select the backend through composition, not conditional sprawl.
5. Document which tests use which backend.

### Recommended implementation
Add a runtime descriptor model such as:

- `RuntimeMode::Headless`
- `RuntimeMode::Playable`

Keep headless mode as the default for:
- tests
- validation
- deterministic scenario harnesses
- CI smoke that does not need a full engine boot

### Acceptance
- `PeterCraftApp` or its replacement can be instantiated against either backend cleanly
- tests still run without the playable engine layer

---

## Step 4 — Define Phase 7 performance budgets before feature work

### Why
If the playable slice starts without budgets, Codex will accidentally optimize for visible progress instead of a shipping-quality feel.

### Required budgets for Phase 7
These are initial targets and must be stored in data/config, not hardcoded across the codebase.

- average frame rate target on target PC hardware
- frame-time p95 budget
- scene transition budget
- cold boot budget
- input-to-motion latency target
- AI decision budget
- save/load budget
- memory budget
- worst-case interaction hitch budget
- audio-voice concurrency budget

### Tasks
1. Create `Phase7PlayableQualityProfile`.
2. Keep it separate from Phase 6 shell quality data.
3. Update telemetry and quality tools to understand the new profile.
4. Add a playability dashboard or summary output for the new metrics.

### Acceptance
- quality gates can report Phase 7-specific budgets
- Codex can fail builds or nightly jobs on regression

---

## Step 5 — Create the playable-slice feature flag map

### Purpose
Prevent the repo from becoming ambiguous during migration.

### Required flags
Codex should define clear runtime feature flags such as:

- `feature.playable_runtime`
- `feature.o3de_adapter`
- `feature.realtime_traversal`
- `feature.realtime_combat`
- `feature.realtime_ai`
- `feature.raid_hud`
- `feature.playable_audio`
- `feature.phase7_blockout_art`

### Rules
- each feature must be testable in isolation where practical
- flags are temporary migration aids, not permanent excuses for dead code
- remove transitional flags once the path is stable

### Acceptance
- debug overlay and logs show active playable-slice flags
- the repo can still boot a deterministic headless run

---

## Step 6 — Strengthen CI and local workflows

### Tasks
1. Keep the current Windows path.
2. Add a secondary compile-check or reduced smoke path for Linux.
3. Add a CI target that verifies the portable core builds without the playable engine backend.
4. Add a “playable build smoke” target once O3DE integration exists.
5. Update build scripts so a new contributor can clearly choose:
   - validate
   - build headless
   - build playable
   - run headless
   - run playable

### Minimum workflow expectation
A contributor must be able to answer:

- How do I build the test runtime?
- How do I build the playable runtime?
- How do I run the one true playable slice?

### Acceptance
- CI is understandable
- commands are explicit
- artifact naming distinguishes headless vs playable outputs

---

## Step 7 — Create the migration board for Phase 7

### Purpose
Protect sequencing.

### Required categories
- build hardening
- runtime separation
- engine integration
- traversal
- combat
- AI
- world/content
- HUD/audio
- quality/playtests

### Rules
- every task must name its dependencies
- every task must name its acceptance criteria
- every task must map to a repo location
- no task should mix engine integration and content polish unless unavoidable

---

## Required file updates in this phase

Codex should expect to update or create:

- `CMakeLists.txt`
- `CMakePresets.json`
- `.github/workflows/ci.yml`
- `.github/workflows/nightly_quality.yml`
- `tools/build-scripts/*`
- `game/code/gems/PeterTelemetry/CMakeLists.txt`
- `game/code/gems/PeterCombat/src/EncounterSimulator.cpp`
- `docs/setup/*`
- a new ADR for runtime separation

---

## Definition of done

This phase is done only when:

- strict-warning builds are green
- platform-specific links are guarded correctly
- headless and playable runtime modes are explicitly separated
- quality budgets for Phase 7 are defined
- CI and local workflows can support the upcoming engine integration
- the team can begin playable work without hidden structural debt

## Failure modes to avoid

Do not:

- disable strict warnings globally
- bury platform checks deep inside game logic
- delete the headless shell
- start O3DE work before the build foundation is green
- let performance goals remain vague
- add art/content before the slice can run reliably
