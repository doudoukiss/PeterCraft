# PeterCraft Codex Planning Pack

This directory contains the canonical step-by-step plans for building **PeterCraft** to a world-class standard.

PeterCraft is a **PC-first, single-player, extraction-adventure maker game** for children, built around a simple idea:

> A child begins by playing a compelling game, then gradually becomes curious about how the game works, and finally starts changing the rules, building content, and writing simple code.

The primary engine assumption is **O3DE**. Unreal Engine remains the fallback if specific decision gates are failed, but all gameplay rules, data formats, and content contracts must be designed to remain portable.

## What Codex should do first

Read these files in order and do not skip ahead:

1. [00_VISION_AND_GUARDRAILS.md](./00_VISION_AND_GUARDRAILS.md)
2. [01_ENGINE_STRATEGY.md](./01_ENGINE_STRATEGY.md)
3. [02_PRODUCT_REQUIREMENTS.md](./02_PRODUCT_REQUIREMENTS.md)
4. [03_TECHNICAL_ARCHITECTURE.md](./03_TECHNICAL_ARCHITECTURE.md)
5. [04_PHASE_0_FOUNDATION.md](./04_PHASE_0_FOUNDATION.md)
6. [05_PHASE_1_VERTICAL_SLICE.md](./05_PHASE_1_VERTICAL_SLICE.md)
7. [06_PHASE_2_CORE_SYSTEMS.md](./06_PHASE_2_CORE_SYSTEMS.md)
8. [07_PHASE_3_AI_AND_COMPANIONS.md](./07_PHASE_3_AI_AND_COMPANIONS.md)
9. [08_PHASE_4_CREATOR_WORKSHOP.md](./08_PHASE_4_CREATOR_WORKSHOP.md)
10. [09_PHASE_5_CONTENT_AND_TOOLS.md](./09_PHASE_5_CONTENT_AND_TOOLS.md)
11. [10_PHASE_6_QUALITY_AND_POLISH.md](./10_PHASE_6_QUALITY_AND_POLISH.md)
12. [11_WORLD_CLASS_SHIP_CRITERIA.md](./11_WORLD_CLASS_SHIP_CRITERIA.md)
13. [12_MILESTONES_AND_BACKLOG.md](./12_MILESTONES_AND_BACKLOG.md)

## Operating rules for Codex

1. **Protect the child-facing experience.**  
   If a technical shortcut would make the game less understandable, less safe, or more frustrating for a child, reject the shortcut.

2. **Prefer clarity over cleverness.**  
   PeterCraft is a teaching product disguised as a premium game. Systems must be elegant, explainable, and debuggable.

3. **Keep gameplay rules engine-agnostic.**  
   Rules, item definitions, quest logic, companion behaviors, and balance tables must live in portable data formats.

4. **Build thin slices, not fake progress.**  
   Every milestone must end in something playable, testable, and measurable.

5. **Instrument everything that matters.**  
   If a feature is introduced without logging, telemetry, debug views, and validation hooks, it is incomplete.

6. **Never ship “magic” to the child.**  
   When the game uses AI, automation, or complex systems, the child should be able to inspect the reasons and outcomes.

7. **No live-service complexity in the first release.**  
   Single-player, offline-first, local saves, child-safe scripting, and zero dependence on backend services except optional update delivery.

## Definition of done for any feature

A feature is not done until all of the following are true:

- implemented and integrated
- data-driven where appropriate
- covered by tests or deterministic validation
- profiled for performance impact
- instrumented for debug and telemetry
- documented for designers
- presented clearly in the child-facing UX
- included in at least one playtest scenario
- reviewed against the world-class quality bar

## Core project promise

PeterCraft must satisfy all three promises at once:

- **Game promise:** the extraction-adventure loop is genuinely fun
- **Learning promise:** the player gradually understands logic and systems
- **Maker promise:** the player can safely change behavior and create content

If any milestone improves one promise while degrading the others, stop and redesign.
