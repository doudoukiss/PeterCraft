# 00. Vision and Guardrails

## Objective

Define the non-negotiable product vision for PeterCraft so every downstream technical and design decision aligns with it.

## Product thesis

PeterCraft is a **single-player salvage-extraction adventure** in a blocky world where the player explores dangerous zones, retrieves resources, escapes with what they find, and uses those resources to upgrade a home workshop.

What makes PeterCraft special is not just the extraction loop. It is that the workshop gradually transforms from a normal upgrade station into a **learning space** where the child begins to inspect rules, reconfigure AI helpers, build missions, and eventually write simple scripts.

The game should feel like:

- a premium adventure game first
- a clear systems toy second
- a gentle introduction to programming third

It must never feel like homework disguised as a game.

## Target audience

### Primary player
- age: around 11
- motivated by curiosity, mastery, collection, and discovery
- may already love Minecraft-like games
- may be intimidated by “real programming”
- needs fast feedback and visible consequences

### Secondary audience
- parents or mentors who want constructive screen time
- teachers, clubs, or after-school programs
- older siblings who may help with maker features

## Design pillars

### 1. Adventure first
The player must want to play even if they never open the creator tools.

### 2. Systems made visible
When the player is curious, the game should reveal why things happen.

### 3. Safe creation
The player can tweak, script, and build without fear of permanently breaking the game.

### 4. Low-friction iteration
Changes must be fast to test. Long compile times or complex toolchains are unacceptable in the child-facing flow.

### 5. World-class craft
The game must feel premium in controls, readability, UX, audio, polish, and stability.

## Player fantasy

The player is a young explorer-engineer recovering lost technology from mysterious dark zones. They are accompanied by robot or creature companions whose behavior can be tuned and upgraded. The deeper the player goes, the more they discover not only treasure, but the logic that governs the world.

## Emotional arc

The core emotional progression should be:

1. **Survival tension** — “Can I make it out?”
2. **Competence** — “I’m getting better at this.”
3. **Curiosity** — “Why did my companion do that?”
4. **Experimentation** — “What happens if I change this rule?”
5. **Creation** — “I made my own mission / AI / level.”

## Non-goals

PeterCraft is **not**:

- an MMORPG
- a social platform
- a competitive PvP game
- a grim military shooter
- a full voxel-sandbox destruction game
- an open-ended coding IDE with no scaffolding
- a monetized platform with pay-to-win progression

## Hard guardrails

1. **Single-player only for v1.**  
   AI replaces teammates. No networking architecture should be allowed to dominate development.

2. **Offline-first.**  
   The game must work without account creation or live backend dependency.

3. **No unrestricted block destruction/building in raids.**  
   Preserve clarity, level design control, performance, and AI reliability.

4. **Child-safe tone.**  
   Tension is welcome. Horror, cruelty, and realistic violence are not.

5. **Loss must teach, not punish.**  
   Extraction risk is important, but setbacks must feel recoverable.

6. **Companion AI must be explainable.**  
   The child should be able to inspect why a companion acted.

7. **Maker tools must be progressively unlocked.**  
   Do not expose the full complexity of professional tools all at once.

## World-class standards

PeterCraft should be benchmarked against premium indie and AA titles, not “educational software.” It must reach a high bar in all of the following:

- movement and camera feel
- combat readability
- UI clarity
- loading times
- save reliability
- crash resistance
- polished audio feedback
- visually coherent art direction
- elegant systems design
- excellent onboarding
- accessible controls
- content pipeline scalability
- maintainable codebase

## Success metrics

Track these as north-star product metrics:

### Player fun
- % of playtesters who voluntarily start a second raid
- average raid completion rate
- average time to first successful extraction
- frustration reports per hour

### Learning
- % of players who open the workshop after 3 sessions
- % of players who modify a behavior rule
- % of players who complete first scripting tutorial
- self-reported confidence around “I can make game logic”

### Creation
- % of players who save a custom mission or companion setup
- average number of user-authored edits per active player
- % of players who revisit creator tools after first use

## Deliverables

Codex must preserve these outputs in all future milestones:

- a design summary that references these pillars
- a feature checklist tagged by pillar
- a quality review template that tests against these guardrails

## Acceptance criteria

This document is considered operational when:

- every phase plan below references the pillars
- all major systems can be tied back to at least one pillar
- every feature request can be rejected or approved using this file
