# 06. Phase 2 - Core Systems

## Objective

Deepen the gameplay systems behind the vertical slice so PeterCraft can support richer content without collapsing under special cases.

## Outputs

- mature combat pipeline
- robust inventory and itemization
- progression scaffolding
- safer loss and recovery systems
- mission variation framework
- stronger tutorial and onboarding logic

## Step-by-step plan

### Step 1: Refactor combat into a clean damage pipeline
Support:
- direct hits
- area effects
- status effects
- friendly support actions
- damage sources and tags
- invulnerability windows where appropriate

Acceptance:
- combat events are logged and debuggable
- every combat outcome can be explained from data

### Step 2: Define item taxonomy
Create explicit categories:
- salvage
- consumables
- tools
- gadgets
- companion modules
- crafting materials
- quest or tutorial items

Acceptance:
- item categories have schema support
- each category has UX rules and inventory rules

### Step 3: Implement rarity and reward clarity
Support:
- rarity definitions
- loot feedback cues
- meaningful choice between risk and reward

Acceptance:
- players can intuit item value without spreadsheet knowledge
- rarity is legible through visuals, audio, and text

### Step 4: Build repair, recovery, and insurance-lite systems
Use a child-friendly version of loss:
- partial durability loss
- recoverable favorite item slot
- low-cost recovery option
- optional safe-mode difficulty assist

Acceptance:
- the game retains tension
- the player never feels that one mistake ruined their progress

### Step 5: Implement mission templates
Create reusable templates such as:
- salvage run
- recover artifact
- activate machine
- escort companion
- timed extraction
- optional side objective

Acceptance:
- mission logic is data-driven
- new missions can be authored without code changes in the common case

### Step 6: Add workshop progression
Create upgrade trees or tracks for:
- player tools
- inventory capacity
- companion capabilities
- creator feature unlocks

Acceptance:
- unlocks are paced and visible
- creator unlocks feel earned, not buried

### Step 7: Improve tutorial architecture
Create a tutorial system that supports:
- contextual prompts
- scripted lesson missions
- workshop lessons
- failure-sensitive hints
- replayable lessons

Acceptance:
- tutorials can be authored from data
- lessons do not require custom code for every new example

### Step 8: Expand accessibility and comfort settings
Support:
- input remapping
- subtitle and text-size controls
- camera sensitivity and comfort
- aim or difficulty assists where appropriate
- reduced time pressure mode

Acceptance:
- core play is accessible without hacking the design

### Step 9: Build post-raid clarity systems
Add:
- raid timeline summary
- what was gained and lost
- companion highlight moments
- lesson or tip from the last run

Acceptance:
- players understand cause and effect after each mission

## Design notes

Core systems must remain elegant. Avoid adding complexity that only experts appreciate. This game is about meaningful systems, not system count.

## Exit criteria

Phase 2 is complete when:
- content designers can author multiple mission variants
- save/load handles richer progression safely
- the penalty model remains motivating and fair
- onboarding is strong enough for first-time players in the target age group
