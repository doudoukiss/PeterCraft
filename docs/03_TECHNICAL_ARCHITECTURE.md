# 03. Technical Architecture

## Objective

Define a maintainable, scalable, world-class architecture for PeterCraft that supports a premium single-player experience and a safe path from play to creation.

## Architectural principles

1. **Local-first and offline-first**
2. **Data-driven gameplay rules**
3. **Engine-agnostic core contracts**
4. **Deterministic, debuggable AI**
5. **Fast iteration for designers**
6. **Safe child-facing extensibility**
7. **Built-in observability and validation**

## High-level runtime architecture

```text
PeterCraft Runtime
├─ Engine Layer (O3DE)
├─ Core Systems Layer
│  ├─ PeterCore
│  ├─ PeterTraversal
│  ├─ PeterCombat
│  ├─ PeterAI
│  ├─ PeterInventory
│  ├─ PeterProgression
│  ├─ PeterWorkshop
│  ├─ PeterWorld
│  └─ PeterUI
├─ Tooling Layer
│  ├─ PeterTelemetry
│  ├─ PeterDebug
│  ├─ PeterValidation
│  └─ PeterTools
└─ Content Layer
   ├─ items
   ├─ missions
   ├─ rooms
   ├─ encounters
   ├─ tutorials
   ├─ companion rules
   └─ creator templates
```

## Module responsibilities

### PeterCore
- game instance lifecycle
- state management
- save/load entry points
- event bus
- versioning and feature flags

### PeterTraversal
- movement
- camera
- jumping, crouching, climbing
- traversal assists
- interaction traces

### PeterCombat
- weapons and gadgets
- damage pipeline
- status effects
- target validation
- hit feedback

### PeterAI
- perception inputs
- navigation integration
- behavior selection
- blackboard/state storage
- explainability logs

### PeterInventory
- item definitions
- inventory containers
- stack rules
- equipment slots
- loot spawn resolution

### PeterProgression
- unlock trees
- workshop unlock state
- upgrade effects
- difficulty assists
- tutorial gating

### PeterWorkshop
- child-facing creator flows
- tuning panels
- rule chips
- script entry point management
- content reset/revert

### PeterWorld
- raid-zone loading
- room assembly
- encounter triggers
- extraction points
- mission objective state

### PeterUI
- HUD
- menus
- workshop UI
- explain panels
- accessibility and localization hooks

### PeterTelemetry
- structured event logging
- performance markers
- playtest export

### PeterDebug
- on-screen debug overlays
- AI decision inspection
- mission state viewers
- room generation viewers

### PeterValidation
- schema validation
- content linting
- save validation
- rule safety checks

### PeterTools
- editor extensions
- batch import/export
- content stamping
- screenshot automation
- test harness utilities

## Repository structure

```text
/petercraft
  /engine-adapters
  /game
    /code
    /gems
    /assets
    /data
    /ui
    /audio
    /tests
  /tools
    /content-validation
    /build-scripts
    /telemetry
    /migration
  /docs
    /adr
    /schemas
    /playtests
```

## Data architecture

All critical content must be represented in portable formats such as JSON, YAML, or other project-approved schema-driven data files.

### Core data domains
- items
- loot tables
- companion rule chips
- mission templates
- room templates
- encounter layouts
- tutorial steps
- script exposure manifests
- workshop unlock tables
- save schema definitions

### Rules
- every data file has a schema
- every schema has validation tests
- every content asset has a stable ID
- migrations are explicit, never implicit

## Save architecture

### Requirements
- local profile save
- rollback-safe save slot system
- versioned save migrations
- sandbox separation between shipped content and user-edited content

### Save domains
- player progression
- companion configuration
- inventory and workshop state
- tutorial state
- creator-made content
- settings and accessibility

### Save rules
- never overwrite the only recoverable save
- maintain at least one backup restore point
- validate before load and after save
- block corrupted or incompatible creator data from crashing the runtime

## AI architecture

Use a layered model:

1. **Perception layer**  
   What the agent currently senses.

2. **Memory / blackboard layer**  
   What the agent remembers, values, and is trying to do.

3. **Decision layer**  
   Utility-based or priority-based action selection.

4. **Action layer**  
   Movement, combat, interaction, revive, guard, follow, retreat.

5. **Explain layer**  
   Human-readable explanation of the last decision and key inputs.

### Why this matters
PeterCraft must teach systems thinking. AI cannot be a black box.

## UI architecture

### Rules
- HUD is minimal and readable
- workshop tools are layered by complexity
- all creator edits use preview + apply + undo
- all teaching flows have “show me why” affordances

### Technical requirements
- separate child-facing strings from internal developer terminology
- support localization-ready string tables
- centralize input prompts and accessibility variants

## Validation architecture

Validation is mandatory and must run:
- pre-commit for changed data
- in CI for all relevant schemas and tests
- in-editor for content authors
- at runtime for user-made content before activation

Validation coverage must include:
- content schemas
- asset references
- navmesh coverage requirements
- mission graph sanity
- loot table sanity
- companion rule safety
- tutorial progression loops
- creator script safety boundaries

## Performance architecture

### Initial target
- 60 FPS on target mid-range Windows PC
- fast cold boot
- sub-second menu transitions where practical
- stable frame pacing
- no spikes caused by child-editable scripts

### Technical strategy
- avoid runtime full-world voxel simulation
- use authored blocky assets and prefabs
- profile every phase
- keep AI update frequency budgeted
- use pooling for frequent effects and pickups
- decouple heavy tools from gameplay runtime

## Testing architecture

### Required test layers
- unit tests for core rules
- integration tests for save/load, inventory, and missions
- deterministic scenario tests for AI
- content validation tests
- smoke tests for level loading and raid completion
- screenshot or capture tests for key UI layouts where feasible

## Documentation architecture

Codex must maintain:
- ADRs for major decisions
- schemas and examples for data authors
- module README files
- feature contracts
- test plans
- playtest reports

## Step-by-step architecture work

1. write ADR for O3DE-first strategy
2. define repository layout
3. create module boundaries
4. define data schemas and stable IDs
5. define save architecture and migration rules
6. define AI explainability contract
7. define validation pipeline
8. define test harness strategy
9. define telemetry event catalog
10. build a reference vertical slice using these boundaries

## Acceptance criteria

The architecture is ready when:
- major systems have named owners and boundaries
- data formats are schema-validated
- no core gameplay system depends directly on child-facing tool code
- save, AI, validation, and telemetry all have clear contracts
- a new engineer can understand the project from the architecture docs alone
