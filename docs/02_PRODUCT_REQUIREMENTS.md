# 02. Product Requirements

## Objective

Define the concrete player-facing requirements for PeterCraft so implementation can proceed without ambiguity.

## Product summary

PeterCraft is a single-player extraction-adventure game with a home workshop and AI companions. The player explores dangerous zones, gathers salvage, fights or evades threats, extracts, upgrades gear and the workshop, and gradually unlocks creator tools.

## Core loop

### Raid loop
1. prepare loadout
2. choose mission
3. enter dark zone
4. explore and collect salvage
5. manage risk, health, and companion behavior
6. reach extraction point
7. return home with recovered items

### Home loop
1. deposit loot
2. craft or upgrade tools
3. tune companion behavior
4. unlock and use creator tools
5. choose next mission

### Learning loop
1. notice a system
2. inspect why it behaved that way
3. tweak a value or rule
4. test the effect
5. build confidence
6. create something new

## Session goals

A standard session should support:
- quick play: 10–15 minutes
- full loop: 20–30 minutes including workshop time
- maker session: 15–45 minutes focused on experimentation

## Vertical-slice feature set

The first real milestone must include:

### Playable
- one home base
- one raid zone
- one extraction flow
- one player character
- one AI companion
- two enemy archetypes
- basic combat and stealth
- loot pickup and carry capacity
- simple crafting
- local save/load

### Learnable
- companion explanation panel
- simple rule-chip editor
- one tutorial that teaches cause and effect

### Buildable
- one safe tuning screen
- one child-facing logic modification exercise
- one “make your own mini mission” path using templates

## Release 1 feature set

### Exploration and combat
- 3 raid zones
- 6–8 mission templates
- 4 enemy archetypes
- 2 companion archetypes
- environmental hazards
- gadgets, tools, and traversal options

### Progression
- workshop upgrades
- companion modules
- item crafting and repair
- cosmetic unlocks from play
- recoverable loss system

### Learning and creation
- tinker mode
- logic mode
- code mode
- replay / explain tools
- mission templates
- saved presets for companions and missions

## Explicit design boundaries

### Permitted
- blocky stylized graphics
- procedural room variation
- deterministic AI
- local saves
- pausing in single-player
- adjustable difficulty
- difficulty assists

### Not permitted in v1
- multiplayer
- PvP
- user-generated content sharing service
- live economy
- chat or social features
- unrestricted scripting access
- realistic firearm focus
- open marketplace monetization

## Player progression model

### Layer 1: character progression
- better survivability
- better salvage efficiency
- better traversal
- better extraction options

### Layer 2: companion progression
- new behaviors
- new priorities
- new support actions
- improved explainability and control

### Layer 3: maker progression
- tune values
- edit rules
- edit logic graphs
- write small scripts

These layers must unlock in parallel so the game naturally guides the player from play to creation.

## Difficulty philosophy

PeterCraft should feel tense but fair.

### Rules
- death and failed extraction should cost something
- the cost must be recoverable
- early game must allow experimentation without severe punishment
- tutorial and creator scenarios must allow reset and retry with no penalty
- enemy readability is more important than raw challenge

## UX requirements

### Essential
- controller and mouse/keyboard support
- large readable UI
- low reading burden for core loop
- visual explanation of important systems
- colorblind-safe signaling
- strong hover/help states
- mistake recovery in creator tools

### Child-facing requirements
- safe defaults
- undo and reset everywhere
- preview before applying changes
- “why” explanations, not only “what”
- no dense jargon in first-time flows

## Audio requirements

Audio must do serious design work:
- enemy telegraphs
- extraction countdown cues
- loot rarity feedback
- companion status feedback
- workshop reward feedback
- creator success feedback

## Narrative requirements

Narrative should be light, supportive, and mysterious:
- encourage exploration
- frame creation as discovery
- avoid dark or cynical themes
- use clear, positive mentor figures when needed

## Save and recovery requirements

- autosave at safe points and key transitions
- explicit backup save slots
- versioned save migration
- sandboxed creator edits
- one-click revert to known-good defaults for child-edited content

## Telemetry requirements

Even in offline mode, log local analytics events for playtest builds:
- raid start and end
- extraction success and failure
- cause of death
- workshop opens
- behavior rule edits
- tutorial completion
- time spent in maker tools
- script validation failures

## Acceptance criteria

The PRD is complete when:
- every release feature is categorized as vertical slice, v1, or later
- every core loop has at least one concrete implementation path
- the learning loop is represented in actual features, not aspirations
- no feature contradicts the vision guardrails
