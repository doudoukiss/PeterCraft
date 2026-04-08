# 07. Phase 3 - AI and Companions

## Objective

Make companions and enemies world-class in clarity, utility, and teaching value.

## Outputs

- robust enemy AI framework
- robust companion AI framework
- explainability tools
- child-facing behavior editing system
- deterministic AI test scenarios

## Design principle

Companion AI is not fake multiplayer. It is the child's first programmable agent.

## AI architecture requirements

### Enemy AI
Must support:
- patrol
- investigate
- alert escalation
- chase
- attack
- retreat or regroup
- return to route

### Companion AI
Must support:
- follow
- scout
- defend player
- loot support
- revive/help
- hold position
- extract assist
- retreat and regroup

## Step-by-step plan

### Step 1: Implement perception
Support:
- line of sight
- hearing or event-based sensing
- interest markers
- threat memory

Acceptance:
- perception is debug-visible in development builds
- designers can inspect why an agent noticed something

### Step 2: Implement blackboard / memory
Store:
- target
- last known threat position
- current goal
- health confidence
- player state
- companion stance
- extraction urgency

Acceptance:
- state transitions are inspectable in debug tools

### Step 3: Implement decision selection
Use a utility or priority system that is:
- deterministic enough for testing
- expressive enough for future content
- explainable in human terms

Acceptance:
- top-ranked action and contributing scores are viewable
- designers can tune weights from data

### Step 4: Implement action library
Create reusable actions:
- move to
- hold
- attack
- evade
- interact
- revive
- loot
- wait
- extract

Acceptance:
- actions are modular and instrumented
- actions can fail gracefully and recover

### Step 5: Build companion stances
Expose stances such as:
- cautious
- balanced
- aggressive
- scavenger
- guardian

Acceptance:
- stances meaningfully change play
- players can feel the difference within one run

### Step 6: Build behavior chips
Behavior chips are child-facing rule modules.

Examples:
- “Stay near me”
- “Protect me first”
- “Grab rare loot”
- “Retreat when hurt”
- “Help at extraction”

Acceptance:
- each chip has plain-language text
- chips map to real AI variables or priorities
- chips can be previewed and reverted

### Step 7: Build explainability UX
Add:
- current goal
- last completed action
- top 2 reasons for current choice
- confidence / risk indicator
- “what changed after your edit?” view

Acceptance:
- a child can successfully answer “why did it do that?”
- the UI avoids overwhelming detail

### Step 8: Build AI test scenarios
Create deterministic test scenes for:
- follow corridor
- enemy ambush
- low health retreat
- revive under pressure
- extraction rush
- loot-vs-safety tradeoff

Acceptance:
- regressions are caught automatically
- bug reports can reference stable scenario IDs

### Step 9: Tune for personality
Companions must feel helpful and alive, but not chaotic. Add:
- brief callouts
- readable gestures
- small idle personality traits
- reliable support patterns

Acceptance:
- companion personality supports learning rather than distracting from it

## Risks

- overusing opaque ML-based AI
- companions overshadowing player skill
- too many editable knobs too early
- explanation UI being technically accurate but unreadable

## Exit criteria

Phase 3 is complete when:
- companions are trusted by playtesters
- enemy behavior feels fair and readable
- behavior edits produce visible differences
- AI debugging is fast for developers and understandable for players
