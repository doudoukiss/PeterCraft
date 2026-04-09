# 17. Phase 7.3 — Combat, AI, and Mission Runtime

## Objective

Turn PeterCraft’s existing combat, AI, and mission logic into a **real-time embodied gameplay loop** that is readable, fair, explainable, and fun.

This phase is where the game earns trust.

## Design rule

Combat must feel tense, but never cruel.

Because PeterCraft is for a child-facing audience and a learning-oriented product, combat must communicate:
- who is threatening the player
- what the player should do next
- what the companion is trying to do
- why success or failure happened

---

## Step 1 — Refactor combat into a runtime-ready architecture

### Current situation
The repository contains deterministic encounter resolution and combat abstractions, but the playable runtime needs embodied combat systems.

### Required runtime layers
- input/action layer
- targeting and hit validation layer
- damage/status effect layer
- feedback layer
- enemy/companion reaction layer
- mission consequence layer

### Tasks
1. Preserve deterministic combat rules in portable code.
2. Add a runtime combat controller that feeds those rules from real-time events.
3. Separate:
   - simulation truth
   - animation timing
   - hit feedback presentation
4. Add runtime combat events that still serialize cleanly into telemetry/debug views.
5. Ensure all damage sources resolve to understandable reasons.

### Acceptance
A combat log or debug panel can explain every meaningful damage event.

---

## Step 2 — Limit the launch combat vocabulary

### Required player tools
For the first playable slice, use a narrow action set:
- one primary tool/weapon
- one secondary or support action
- one heal/recovery consumable
- one contextual interaction

### Why
A small, well-tuned kit is better than a messy arsenal.

### Combat feel goals
- telegraphed enemy attacks
- readable hit confirmation
- reliable dodge/spacing opportunities if movement supports it
- low ambiguity on healing and support actions

---

## Step 3 — Implement two enemy archetypes only

### Archetype A — Patrol guard
Purpose:
- basic route pressure
- readable engagement
- entry-level combat literacy

Expected behaviors:
- patrol
- notice
- approach
- attack
- retreat or reset
- call out alert state

### Archetype B — Pressure striker or ranged suppressor
Purpose:
- create timing pressure near loot or extraction
- force the player to reposition
- encourage the companion to reveal its value

Expected behaviors:
- hold or pressure space
- punish static play
- interact with extraction tension

### Rules
- enemy differences must be felt in under 10 seconds
- every enemy action must have a readable cue
- enemy failures must be inspectable in debug mode

### Acceptance
Testers can describe the two enemy types accurately after one session.

---

## Step 4 — Turn the companion into a real-time partner, not a fake player

### Companion role in Phase 7
The companion should:
- follow
- defend
- revive/help
- loot opportunistically
- call out useful information
- bias toward extraction safety near the end of a run

### Do not allow the companion to
- dominate combat
- trivialize navigation
- make the player feel unnecessary
- act unpredictably without explanation

### Tasks
1. Convert the existing decision logic into a runtime tick/update model.
2. Use navigation and perception inputs from the playable world.
3. Maintain the explain snapshot model.
4. Create a simple companion-command surface if required:
   - follow
   - hold
   - focus threat
   - focus loot
5. Ensure companion behavior remains deterministic enough for debugging.

### Acceptance
Players describe the companion as “helpful” and “understandable,” not “random” or “too strong.”

---

## Step 5 — Keep explainability visible in the playable runtime

### Why
This is one of PeterCraft’s differentiators.

### Required explain outputs
- current state
- current goal
- last major action
- top reason
- strongest modifiers
- recent perception event
- current target
- caution/extraction bias when relevant

### UX rule
The explain panel must be simple enough for a child and detailed enough for a designer.

### Tasks
1. Port existing explain text/panel work into the real UI layer.
2. Add quick open access in pause or companion terminal flows.
3. Add a lighter-weight “companion callout” version during active play.
4. Keep development-only deep debug data available.

### Acceptance
A child can answer one “why did the companion do that?” question correctly.

---

## Step 6 — Build the mission runtime state machine

### Required mission states
- not started
- active
- objective in progress
- pressure beat triggered
- extraction available
- extraction started
- success
- failure
- post-raid results committed

### Tasks
1. Turn mission templates/blueprints into real runtime objective graphs.
2. Keep mission truth in portable structures where practical.
3. Ensure every state transition emits telemetry.
4. Separate mission logic from scene-loading logic.
5. Make failure reasons explicit.

### Required failure reasons
- player defeated
- extraction interrupted
- timer expired
- objective failed
- manual abort if designed later

### Acceptance
The current mission can be debugged from a readable state machine view.

---

## Step 7 — Make loot, risk, and persistence emotionally coherent

### Required loop
- find loot
- choose what matters
- carry risk
- survive pressure
- extract
- persist results safely

### Tasks
1. Keep item IDs, rarity, and inventory rules portable.
2. Spawn loot in readable, authored anchor points.
3. Ensure loot excitement is visible in world and HUD feedback.
4. Preserve loss/recovery logic from the existing inventory and progression systems.
5. Save only at safe points.

### Acceptance
A player can tell the difference between low-value salvage and meaningful loot immediately.

---

## Step 8 — Preserve parity between runtime and headless validation

### Why
The playable runtime must not fork away from the trusted logic base.

### Tasks
1. Add tests that compare runtime-facing combat/AI configuration against deterministic harness outputs where possible.
2. Keep a small set of golden scenarios for companion behavior.
3. Add replay or event-log exports for real play sessions.
4. Use these exports in bug repro workflows.

### Acceptance
A companion or mission regression found in the playable slice can be reproduced in tooling.

---

## Required repository changes

Codex should expect to update or add:

- `game/code/gems/PeterCombat/*`
- `game/code/gems/PeterAI/*`
- `game/code/gems/PeterInventory/*`
- `game/code/gems/PeterProgression/*`
- `game/code/gems/PeterWorld/*`
- `game/code/gems/PeterUI/*`
- mission and encounter content data
- debug overlay and replay views
- new runtime and parity tests

---

## Definition of done

This phase is done only when:

- the player can fight in real time
- two enemy archetypes exist and are meaningfully different
- the companion helps in real time and remains explainable
- mission success and failure resolve correctly
- loot and extraction create understandable risk
- post-raid persistence remains safe and trustworthy
- headless parity coverage still exists for the most important systems

## Failure modes to avoid

Do not:

- let animation hide simulation truth
- let the companion become overpowering
- let combat ambiguity rise because debugability fell
- expand enemy count before two archetypes are excellent
- add deep loot complexity before the core risk/reward loop is readable
