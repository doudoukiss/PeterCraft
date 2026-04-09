# O3DE Runtime Spike

## Purpose

Track the measured O3DE evidence as PeterCraft moves from the Phase 7.1 host baseline into the Phase 7.2 playable slice.

## Required measurements

1. time to first playable room
2. time to add one interactable
3. time to add one enemy archetype
4. time to author one blockout room
5. time to debug one AI route failure
6. build reproducibility
7. average iteration time for a gameplay-parameter change

## Current evidence sources

- generated build summary: `Saved/Generated/o3de/playable_build_summary.md`
- generated runtime summary: `Saved/Generated/o3de/playable_runtime_report.md`
- adapter log: `Saved/Logs/petercraft-o3de-adapter.log`

## Current baseline

- engine version: `25.10.2`
- project root: `game/o3de/`
- one-room proof scene id: `scene.playable.one_room_proof`
- current playable raid scene id: `scene.playable.raid.machine_silo`
- current playable home scene id: `scene.playable.home_base`
- current playable results scene id: `scene.playable.results`

## Gate rule

Stay on O3DE unless the measured evidence shows a real throughput, stability, or performance failure against PeterCraft's concrete needs.
