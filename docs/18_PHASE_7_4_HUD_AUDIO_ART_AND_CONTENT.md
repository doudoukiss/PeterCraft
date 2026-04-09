# 18. Phase 7.4 — HUD, Audio, Art, and Content for the Playable Slice

## Objective

Transform the mechanically playable slice into a **clear, premium, child-friendly experience**.

This phase is not “final art.”  
It is the first time PeterCraft should feel intentional, readable, and emotionally complete.

## Rule

Everything added in this phase must improve one of these:

- clarity
- responsiveness
- trust
- emotional reward
- teaching value

If it only adds decoration, delay it.

---

## Step 1 — Build the slice HUD

### Required HUD elements
- player health/state
- companion status
- objective summary
- extraction status/countdown
- interact prompt
- loot pickup feedback
- inventory quick state
- accessibility-safe signal redundancy
- optional explain-entry point

### UX rules
- the player should never need to guess the current objective
- color must not be the only signal
- threat and extraction signals must be different
- prompts must be quiet until needed
- the HUD must not look like a debug prototype once Phase 7 closes

### Tasks
1. Convert `PeterUI` from panel text rendering toward actual playable UI widgets.
2. Keep child-facing language simple and supportive.
3. Maintain a debug overlay separate from the player-facing HUD.
4. Respect accessibility settings already present in the repo.

### Acceptance
Players can describe their goal and status without opening a menu.

---

## Step 2 — Create a disciplined blockout art kit

### Goal
The slice must look like an intentional blocky world, not random cubes.

### Required kit categories
- floor types
- wall types
- support/trim pieces
- stairs and ramps
- doors/gates
- interactable silhouettes
- loot container silhouettes
- mission-object silhouettes
- extraction landmark language
- home-base station language

### Art rules
- every interactable must be visually distinct
- mission-critical objects must silhouette clearly from gameplay camera distance
- the home base and raid zone must use different mood signatures
- landmarking must work even without texture complexity

### Tasks
1. Build a minimal greybox/voxel-inspired kit.
2. Define scale guides and silhouette rules.
3. Document landmark patterns for all playable rooms.
4. Add a content-review checklist focused on readability.

### Acceptance
Screenshots of the slice are readable even without annotations.

---

## Step 3 — Add lighting and atmosphere that teach space

### Home base
Should feel:
- safe
- warm
- understandable
- invitational

### Raid zone
Should feel:
- tense
- mysterious
- readable
- navigable

### Tasks
1. Establish a lighting bible for the slice.
2. Use lighting to indicate:
   - safe paths
   - optional danger
   - extraction
   - mission-critical space
3. Avoid darkness that causes confusion.
4. Add comfort settings where relevant.

### Acceptance
Players orient using space, not only UI.

---

## Step 4 — Add a first premium audio pass

### Required cue categories
- movement surface feedback
- interaction confirm/deny
- loot rarity pickup
- threat detected
- enemy attack telegraph
- companion assist callout
- extraction start
- extraction success/failure
- workshop success/failure
- results screen emotional reinforcement

### Audio rules
- sound must clarify state changes
- repetition fatigue must be managed
- companion audio must support trust, not annoyance
- workshop feedback must feel rewarding and safe

### Tasks
1. Create a cue map aligned to existing cue IDs where possible.
2. Implement real playback through the audio adapter.
3. Add priority and concurrency behavior for overlapping moments.
4. Test with muted UI to see whether sound alone improves clarity.

### Acceptance
Key state changes remain understandable even when the player is not looking directly at the relevant object.

---

## Step 5 — Make the first 10 minutes teach naturally

### Required onboarding beats
- move
- interact
- understand mission start
- understand loot
- survive first fight
- notice companion help
- recognize extraction
- understand post-raid result
- perform one safe workshop change

### Design rules
- avoid long text dumps
- teach through space and action first
- use short prompts only when necessary
- preserve player dignity after failure

### Tasks
1. Re-evaluate existing tutorial lessons against the new real-time slice.
2. Convert only the essential lessons into embodied guidance.
3. Add onboarding telemetry for:
   - first movement
   - first interact
   - first damage taken
   - first loot
   - first extraction attempt
   - first success/failure
   - first workshop edit

### Acceptance
A new player reaches extraction without feeling tutorialized to death.

---

## Step 6 — Narrow content to world-class quality

### Rule
Do not expand breadth during this phase.

### Required content target
- one home base polished enough to navigate naturally
- one raid zone polished enough to replay
- one mission polished enough to feel intentional
- two enemies polished enough to be learnable
- one companion polished enough to trust

### Tasks
1. Review every room in the slice for purpose.
2. Remove filler.
3. Add one optional risk/reward detour.
4. Add one memorable landmark encounter beat.
5. Add one recovery-safe failure path.

### Acceptance
The slice is compact, but replayable and memorable.

---

## Step 7 — Add accessibility and child-facing polish

### Required checks
- text scaling
- readable prompts
- color redundancy
- reduced time-pressure mode
- remappable input
- subtitle/readability support if text/audio is used
- non-hostile failure messaging

### Tasks
1. Validate the new HUD and prompts against the repo’s accessibility standards.
2. Make every critical state understandable without fast reading.
3. Audit the child-facing tone in all new text.

### Acceptance
The playable slice remains kind, readable, and fair.

---

## Required repository changes

Codex should expect to update or add:

- `game/ui/*`
- `game/audio/*`
- `game/assets/*`
- `game/code/gems/PeterUI/*`
- `docs/quality/*`
- content review records for slice rooms/missions/encounters
- onboarding telemetry hooks
- audio and HUD validation scripts/checklists

---

## Definition of done

This phase is done only when:

- the slice has a real HUD
- audio meaningfully supports play
- the blockout art is intentional and readable
- the first 10 minutes teach through play
- the slice feels premium even without final art
- accessibility and child-facing tone remain intact

## Failure modes to avoid

Do not:

- hide readability under moody lighting
- add too many UI widgets
- rely on text instead of spatial teaching
- attempt final-art ambition before slice quality is proven
- widen the slice when the current content still needs polish
