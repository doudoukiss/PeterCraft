# 01. Engine Strategy

## Objective

Choose the engine strategy for PeterCraft and define the decision rules that keep the project portable and world-class.

## Primary recommendation

Use **O3DE** as the primary engine for the initial PC-first build.

## Why O3DE is the default choice

1. **Open-source alignment**  
   PeterCraft is partly an educational product. Open ownership of the engine and toolchain aligns with that mission.

2. **Good fit for the intended platform**  
   PeterCraft v1 is single-player and PC-first. This reduces pressure on mobile deployment and large-scale runtime networking.

3. **Strong conceptual ladder for learning**  
   A clean path from data editing to visual scripting to Lua scripting fits the product vision.

4. **Modular architecture**  
   Gems and clear engine modules support long-term maintainability.

5. **Lower dependence on marketplace assets**  
   The blocky art direction reduces the need for a huge photoreal asset ecosystem.

## Why Unreal remains the fallback

Unreal becomes the fallback if one or more of the following become dominant constraints:

- fast mobile or tablet deployment becomes mandatory
- the team needs much stronger out-of-the-box AI tooling
- editor ergonomics block content throughput
- O3DE build, tooling, or iteration speed becomes a bottleneck
- production velocity is consistently harmed by missing ecosystem support

## Engine decision protocol

Codex must run a formal decision spike before locking the engine.

### Step 1: Define the evaluation scope
Build the same tiny prototype in the chosen primary engine path, then evaluate whether a fallback investigation is needed.

Prototype contents:
- first-person or over-the-shoulder movement
- one small blocky raid room
- one enemy
- one AI companion
- one loot interaction
- one extraction point
- one simple rule-editing panel

### Step 2: Measure the following
- time to first playable prototype
- time to hot-iterate on gameplay values
- time to author a new room
- AI debugging ergonomics
- build reproducibility
- performance on target PC hardware
- onboarding cost for new engineers and designers

### Step 3: Decide
Remain on O3DE unless there is hard evidence that one of the decision gates has failed.

## Decision gates that trigger an Unreal re-evaluation

Trigger a formal re-evaluation if any of these happen:

1. **Iteration-speed failure**  
   A simple gameplay change routinely takes too long to test.

2. **Tooling failure**  
   Designers cannot reliably author missions, encounters, or AI data without engineering intervention.

3. **Performance failure**  
   Target frame rate or loading budgets are missed by a margin that seems systemic rather than fixable.

4. **Pipeline failure**  
   Importing, validating, and shipping assets becomes unstable or costly.

5. **Talent failure**  
   Hiring or onboarding becomes materially slower because the stack is too niche.

## Portability rules

Even when using O3DE, build the project as though it may need to migrate later.

### Keep these engine-agnostic
- item definitions
- loot tables
- mission data
- dialogue and tutorial data
- behavior-chip data
- companion decision weights
- room generation metadata
- save schema version definitions
- balance spreadsheets exported to portable formats

### Keep these behind adapters
- input bindings
- save-system backend
- navmesh integration
- audio playback triggers
- UI widgets
- animation event wiring
- camera systems

### Do not lock game rules into engine-only scripts
Core rules must live in plain data plus portable gameplay logic. Visual scripting should orchestrate content, not contain the irreplaceable heart of the game.

## O3DE implementation strategy

### Engine layer
- use O3DE directly, pinned to a project-approved version
- track local engine patches separately
- document all engine modifications

### Game layer
Create the following major Gems or modules:
- `PeterCore`
- `PeterTraversal`
- `PeterCombat`
- `PeterAI`
- `PeterInventory`
- `PeterWorkshop`
- `PeterWorld`
- `PeterUI`
- `PeterTelemetry`
- `PeterTools`

### Child-safe scripting layer
Expose only a controlled subset of editable variables and scripts to the child-facing workshop kit. Never expose raw engine APIs directly.

## Unreal fallback strategy

If a formal re-evaluation is triggered, do not migrate immediately. Instead:

1. freeze new feature development for one sprint
2. identify the exact failure modes
3. build a parity check in Unreal for the failing area only
4. compare cost of fixing O3DE issues vs porting
5. migrate only if the long-term cost model justifies it

## Deliverables

Codex must create and maintain:
- an engine ADR (architecture decision record)
- a portability checklist
- a list of engine patches and local tooling additions
- a decision-gate dashboard

## Acceptance criteria

This strategy is complete when:
- the project has a written engine ADR
- all gameplay data formats are portable
- all major engine dependencies are behind adapters
- the decision gates are measurable, not subjective
