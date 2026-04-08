# 04. Phase 0 - Foundation

## Objective

Create a clean, scalable development foundation for PeterCraft before building major gameplay systems.

## Inputs

- vision and guardrails
- engine strategy
- product requirements
- technical architecture

## Outputs

- compilable project skeleton
- repository conventions
- CI pipeline
- validation pipeline
- module scaffolding
- coding standards
- editor setup
- test harness shell
- first bootable empty scene

## Step-by-step plan

### Step 1: Create the repository structure
Create the top-level folders defined in the architecture doc.

Deliverables:
- `/game`
- `/tools`
- `/docs`
- `/tests`
- `/docs/adr`

Acceptance:
- clean checkout builds the baseline project
- directories have README files describing purpose

### Step 2: Write core ADRs
Write architecture decision records for:
- engine strategy
- module boundaries
- data format strategy
- save philosophy
- scripting safety model

Acceptance:
- each ADR has context, decision, consequences, and open questions

### Step 3: Establish coding standards
Define:
- C++ style guide
- naming conventions
- data schema naming
- folder ownership rules
- test naming rules
- commit hygiene expectations

Acceptance:
- style rules are machine-checkable where possible
- lint errors fail CI

### Step 4: Set up build and CI
Create:
- local build script
- CI build for main target platform
- lint and validation step
- test execution step
- artifact packaging for test builds

Acceptance:
- pull requests cannot merge unless build, lint, and tests pass

### Step 5: Scaffold core modules
Create stub Gems/modules for:
- PeterCore
- PeterTraversal
- PeterCombat
- PeterAI
- PeterInventory
- PeterProgression
- PeterWorkshop
- PeterWorld
- PeterUI
- PeterTelemetry
- PeterDebug
- PeterValidation
- PeterTools

Acceptance:
- each module builds
- each module has README, owner, dependency list, and public API placeholder

### Step 6: Define the event bus and logging contract
Create a minimal structured event system.

Required event categories:
- gameplay
- AI
- save/load
- creator tools
- validation
- performance markers

Acceptance:
- one event can be emitted, viewed locally, and written to file

### Step 7: Define data schemas
Create initial schemas for:
- items
- loot tables
- rooms
- missions
- companion rules
- tutorial steps
- save schema version metadata

Acceptance:
- schema validation script exists
- example files validate successfully
- intentionally broken files fail as expected

### Step 8: Create the test harness
Add:
- unit test target
- integration test target
- deterministic scenario harness
- smoke test scene boot

Acceptance:
- tests can run in CI and locally
- one passing test exists in each category

### Step 9: Build the empty runtime shell
Implement:
- boot flow
- main menu placeholder
- settings placeholder
- scene loading shell
- local profile creation stub

Acceptance:
- app launches to a stable menu
- user can create a profile and load an empty scene

### Step 10: Add core debug overlays
Implement toggles for:
- FPS and frame time
- active scene and mission ID
- current player state
- active companion state
- save slot state

Acceptance:
- overlays are available in development builds without code edits

## Non-goals for Phase 0

Do not build:
- final combat
- final AI
- final workshop UX
- content-heavy maps
- advanced art
- unnecessary engine customizations

## Risks

- over-engineering the foundation
- hiding architectural decisions in code rather than ADRs
- weak validation that will not scale
- unclear module dependencies

## Exit criteria

Phase 0 is complete only when:
- a new developer can clone, build, run, and understand the project
- validation exists for the first data files
- the project can boot into an empty playable shell
- all future work can be added without structural rework
