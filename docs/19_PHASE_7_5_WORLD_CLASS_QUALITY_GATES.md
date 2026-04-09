# 19. Phase 7.5 — World-Class Quality Gates for the Playable Slice

## Objective

Prove that the new playable PeterCraft slice meets a world-class standard across:

- engineering quality
- design quality
- control feel
- AI trust
- save integrity
- onboarding clarity
- child-facing educational value

This phase exists so the team does not confuse “it runs” with “it is ready.”

## Core rule

A playable slice is not complete when it is visible.  
It is complete when it is **repeatably good**.

---

## Gate 1 — Engineering trust

### Required evidence
- green CI
- repeatable build instructions
- deterministic headless tests still passing
- playable runtime smoke path working
- no high-severity unresolved crashers
- no known save corruption path

### Required actions
1. Add a Phase 7 smoke test for boot-to-playable-room.
2. Add automated checks for:
   - boot
   - scene transition
   - save/load
   - mission launch
   - results return
3. Keep parity tests for portable logic where practical.

### Fail condition
If a new contributor cannot build and run the slice predictably, do not proceed.

---

## Gate 2 — Performance and frame stability

### Required measurements
- cold boot time
- scene transition time
- average FPS
- p95 frame time
- input-to-motion latency
- peak working set memory
- AI decision timing
- save/write hitch time
- worst interaction hitch

### Tasks
1. Capture budgets from the Phase 7 quality profile.
2. Add automated perf capture where practical.
3. Run multi-pass measurements on target PC hardware.
4. Capture results in a shareable report.

### Fail condition
If frame pacing or interaction hitching regularly breaks the feel of the game, do not claim playability.

---

## Gate 3 — Save integrity and profile safety

### Required proof
- fresh profile works
- existing profile upgrades safely
- save after success works
- save after failure works
- workshop edit persists correctly
- crash/interrupt recovery path is safe
- corrupted creator content cannot brick the profile

### Tasks
1. Run save-health verification after every critical transition.
2. Add fault-injection tests if feasible.
3. Add manual QA steps for interrupted writes and backup restores.
4. Verify home/raid/results loops across repeated sessions.

### Fail condition
Any profile-loss or progression-loss path is stop-ship.

---

## Gate 4 — Control and camera feel

### Required proof
- new players can move confidently
- camera does not fight the player near walls or interactables
- controller is viable
- mouse/keyboard is viable
- comfort options work

### Playtest questions
- Did movement feel responsive?
- Did the camera ever annoy you?
- Did you know where to go?
- Did you feel in control during combat?

### Fail condition
If testers complain about controls before they talk about the game itself, the feel bar is not met.

---

## Gate 5 — Combat readability and fairness

### Required proof
- threat telegraphs are readable
- damage sources are understandable
- healing/recovery is understandable
- extraction pressure feels fair
- failure does not feel random

### Metrics
- deaths by cause
- deaths near extraction
- first-combat survival rate
- second-attempt improvement rate

### Fail condition
If players cannot explain why they lost, combat/failure clarity is not ready.

---

## Gate 6 — Companion trust and explainability

### Required proof
- players notice the companion helping
- players do not feel the companion steals the game
- players can explain one major companion choice
- explain UI is used and understood

### Metrics
- explain panel open rate
- companion trust rating
- revive/help success rate
- companion intervention moments per run

### Fail condition
If the companion feels random, annoying, or invisible, this gate fails.

---

## Gate 7 — First-run onboarding and educational bridge

### Required proof
- a child can reach the first mission
- a child can complete or meaningfully attempt the first extraction
- the player notices one causal relationship between choice and result
- the workshop change is understandable and safe

### Metrics
- first mission launch rate
- first extraction attempt rate
- first extraction success rate
- first workshop edit rate
- second-run completion rate

### Fail condition
If the workshop feels disconnected from the gameplay loop, PeterCraft’s promise is not being delivered.

---

## Gate 8 — Content and presentation coherence

### Required proof
- the home base is navigable
- the raid zone is memorable
- landmarks work
- audio supports state changes
- HUD is clear and not noisy
- the blockout art feels intentional

### Review method
Run a cross-discipline review with:
- design
- engineering
- UI/UX
- audio
- QA

### Fail condition
If the slice still feels like separate prototypes glued together, this gate fails.

---

## Required playtest plan

### Cohort A — Adult internal first-pass
Purpose:
- catch usability and obvious bugs quickly

### Cohort B — Non-dev adults
Purpose:
- identify unspoken assumptions from the team

### Cohort C — Child-facing target players with supervision
Purpose:
- validate clarity, fairness, and educational curiosity

### Required session structure
1. 5-minute intro
2. unassisted play
3. short interview
4. second run with one workshop change
5. follow-up questions on companion behavior and mission understanding

### Required questions
- What were you trying to do?
- What made you choose that route?
- What did your companion do that helped?
- Why did you succeed or fail?
- What changed after your workshop edit?

---

## Required artifacts

Codex must produce or update:

- playtest script
- playtest score sheet
- save-health report
- performance report
- AI trust summary
- onboarding funnel report
- issue triage output
- Phase 7 release-readiness checklist

---

## Exit criteria for Phase 7

Phase 7 can close only when all of the following are true:

- the slice is genuinely playable
- the slice is stable
- the slice is teachable
- the slice is trustworthy
- the slice is fun enough to replay
- the slice still respects the architecture and safety promises of PeterCraft

## Failure modes to avoid

Do not:

- treat internal confidence as evidence
- skip child-facing validation
- hide behind “it’s just blockout”
- waive save bugs because the slice is still small
- accept “almost” on control feel or companion trust
