# 05. Phase 1 - Vertical Slice

## Objective

Build the smallest version of PeterCraft that proves the entire core loop: prepare, raid, extract, return, upgrade, inspect, and tweak.

## Inputs

- foundation complete
- schemas and module scaffolds in place

## Outputs

- one playable home base
- one compact raid zone
- one extraction flow
- one AI companion
- one enemy family
- basic loot and crafting
- save/load between home and raid
- one child-facing rule-edit interaction

## Vertical-slice definition

A player should be able to:
1. launch the game
2. load a profile
3. enter the home base
4. inspect loadout
5. enter a raid
6. collect loot
7. fight or avoid danger
8. extract successfully or fail
9. return home
10. craft or upgrade one thing
11. inspect why the companion made a decision
12. change one behavior rule and see the impact in the next raid

## Step-by-step plan

### Step 1: Implement movement and camera
Build:
- walking
- sprinting
- jumping
- crouching
- interaction trace
- readable camera tuning

Acceptance:
- movement feels stable and responsive
- controller and mouse/keyboard both work
- motion is comfortable for younger players

### Step 2: Implement interaction framework
Build:
- interactable base class or contract
- lootable object
- door or gate
- extraction switch or zone trigger
- workshop station interaction

Acceptance:
- all interactions share consistent prompts and feedback
- interactions support hover/help text

### Step 3: Build the compact home base
Include:
- stash terminal
- crafting/workbench
- companion terminal
- mission board
- clear visual landmarks

Acceptance:
- home base loads quickly
- every station is reachable and understandable
- one loop from raid return to next launch works without confusion

### Step 4: Build the compact raid zone
Requirements:
- 5 to 8 authored rooms
- 2 optional side paths
- 1 high-risk reward room
- 1 extraction point
- clear entrance and exit readability

Acceptance:
- raid can be completed in 8–12 minutes
- navmesh supports AI in all intended spaces
- room metrics and IDs are debug-visible

### Step 5: Implement basic loot and inventory
Build:
- item pickup
- stack rules
- carry capacity
- quick-view loot summary
- extraction transfer to home stash

Acceptance:
- loot is readable and satisfying to collect
- failed extraction resolves item outcomes consistently
- save/load preserves inventory state correctly

### Step 6: Implement the fail/escape loop
Build:
- raid success state
- raid fail state
- extraction countdown or hold interaction
- simple recoverable penalty model

Acceptance:
- success and failure are clearly communicated
- the player understands what was kept and what was lost
- the system encourages replay rather than discourages it

### Step 7: Add one enemy family
Recommended initial enemy family:
- patrol unit
- alert on sight or sound
- chase, attack, retreat, reset

Acceptance:
- telegraphs are clear
- combat is understandable
- failure feels fair

### Step 8: Add one companion
Companion minimum actions:
- follow
- attack same target
- revive or help
- hold position
- retreat when unsafe

Acceptance:
- companion is useful but not overpowering
- the player can tell what the companion is trying to do

### Step 9: Add companion explainability
Implement a simple panel that shows:
- current state
- last action
- top decision reason
- one or two influential variables

Acceptance:
- a child can read the panel and form a correct mental model
- internal jargon is hidden

### Step 10: Add one behavior edit
Expose one safe rule such as:
- follow distance
- loot priority
- aggression threshold

Acceptance:
- change can be previewed, applied, and reverted
- the effect is visible within one raid

### Step 11: Add simple crafting or upgrade
At minimum:
- turn recovered salvage into one tool, heal item, or companion upgrade

Acceptance:
- crafting is rewarding and clearly explained
- crafted output is usable in the next raid

### Step 12: Add first-time user experience
Include:
- a short playable tutorial
- contextual hints
- clear success feedback
- post-raid summary

Acceptance:
- a new player can complete the full loop with minimal help

## Required instrumentation

Track:
- raid start/end
- extraction success/failure
- cause of failure
- items recovered
- companion actions
- behavior edit usage
- crafting usage

## Non-goals for Phase 1

Do not add:
- multiple maps
- advanced procedural generation
- advanced narrative systems
- complex scripting
- content volume beyond what is needed to validate the loop

## Exit criteria

Phase 1 is complete only when playtesters can say:
- “I understand the loop.”
- “I want to try another run.”
- “I can tell what my companion is doing.”
- “Changing that rule actually changed the game.”
