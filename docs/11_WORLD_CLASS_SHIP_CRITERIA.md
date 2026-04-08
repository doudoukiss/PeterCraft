# 11. World-Class Ship Criteria

## Objective

Define the quality bar PeterCraft must hit before any release candidate can be considered shippable.

## Core rule

PeterCraft does not ship because the backlog is exhausted. It ships because the experience is coherent, premium, stable, and meaningfully educational.

## Stop-ship principle

Any one of the following can block release:
- unreliable save behavior
- confusing or unfair core combat
- workshop flows that can break player progress
- inaccessible critical UI
- poor performance on target hardware
- unclear onboarding for target-age players
- AI companions that feel untrustworthy or inexplicable

## Ship bar by discipline

### 1. Player experience

Required:
- the first 15 minutes are understandable
- the first extraction can be achieved by a new player
- failure feels fair and recoverable
- the second session is better than the first, not more confusing

Evidence:
- moderated playtests
- post-session interview notes
- completion and retry metrics

### 2. Controls and camera

Required:
- movement feels responsive and intentional
- camera never fights the player
- controller and mouse/keyboard are both viable
- comfort options exist for camera-sensitive players

Evidence:
- input-mode QA checklist
- playtest observations
- bug counts for control frustration

### 3. Combat and risk/reward loop

Required:
- enemy telegraphs are readable
- damage sources are understandable
- extraction risk creates tension without cruelty
- loot is exciting but legible

Evidence:
- combat review sessions
- heatmaps or logs for deaths
- player explanations of combat outcomes

### 4. AI and companions

Required:
- companions are helpful but not dominant
- players can predict broad companion behavior
- “why did it do that?” has a clear answer
- enemy AI feels fair, varied, and reliable

Evidence:
- AI test scenarios
- companion explanation UX reviews
- trust scores from playtest feedback

### 5. Workshop and creator tools

Required:
- tinker mode is safe and inviting
- logic mode teaches cause and effect
- code mode has helpful errors and cannot crash the game
- all creator flows support undo, reset, and preview

Evidence:
- workshop completion funnel
- script validation error logs
- child playtests focused on creator tasks

### 6. Educational value

Required:
- players move from play to curiosity naturally
- the game teaches systems thinking through interaction
- at least one tiny scripting success is achievable
- the language used is supportive and age-appropriate

Evidence:
- tutorial progression metrics
- child interview notes
- creator-task success rate

### 7. Visual quality

Required:
- blocky art is stylized, not cheap
- silhouettes and landmarks are readable
- reward objects stand out appropriately
- VFX support play clarity rather than overwhelm it

Evidence:
- art reviews
- readability captures
- level review checklists

### 8. Audio quality

Required:
- key game states are communicated through sound
- music and ambience support tension and relief
- feedback sounds are polished and consistent
- creator tools have satisfying success and validation cues

Evidence:
- audio pass reviews
- UX feedback logs
- comparison against silent or degraded builds

### 9. Performance and stability

Required:
- target frame rate is met consistently
- load times are reasonable
- no major hitching in core gameplay
- crashes are rare and actionable
- save/load is resilient

Evidence:
- performance captures
- crash analytics for test builds
- long-session soak tests

### 10. Accessibility

Required:
- text is readable
- color is not the only signal
- inputs are remappable
- time pressure can be eased where necessary
- critical flows are navigable for a broad set of players

Evidence:
- accessibility checklist
- targeted review passes
- bug and issue tracking by category

### 11. Engineering quality

Required:
- architecture remains modular
- tests cover critical systems
- validation protects content quality
- debugging tools exist for every major system
- docs are current enough for new contributors

Evidence:
- CI status
- test coverage summaries
- ADR history
- module documentation audits

### 12. Content pipeline quality

Required:
- new rooms and missions can be authored predictably
- validation catches common mistakes
- content review is disciplined
- adding content does not require heroics

Evidence:
- authoring trial runs
- validation reports
- content production retrospectives

## Final release checklist

PeterCraft is release-ready only when all answers below are “yes”:

- Would a child want to play again after a failed run?
- Can a child explain one companion decision correctly?
- Can a child safely change a rule and see the effect?
- Can the game survive bad custom content without damage?
- Does the experience feel premium in movement, audio, UI, and feedback?
- Can the team build and validate the project repeatably?
- Is the game fun even for players who ignore the educational layer?
- Is the educational layer meaningful rather than superficial?

## If the answer is “almost”

“Almost” is not a ship state. If a major category is not at bar, delay, cut scope, or redesign. Protect the standard.
