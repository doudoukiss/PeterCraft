# O3DE Runtime Spike

## Purpose

Track the measured Phase 7.1 O3DE evidence before broader playable feature work expands.

## Required measurements

1. time to first playable room
2. time to add one interactable
3. time to add one enemy archetype
4. time to author one blockout room
5. time to debug one AI route failure
6. build reproducibility
7. average iteration time for a gameplay-parameter change

## Current evidence sources

- generated build summary: `Saved/Generated/o3de/phase7_1_build_summary.md`
- generated runtime summary: `Saved/Generated/o3de/phase7_1_runtime_spike.md`
- adapter log: `Saved/Logs/petercraft-o3de-adapter.log`

## Phase 7.1 baseline

- engine version: `25.10.2`
- project root: `game/o3de/`
- one-room proof scene id: `scene.playable.one_room_proof`
- current playable raid scene id: `scene.raid.machine_silo`
- current playable home scene id: `scene.home.workshop_yard`

## Gate rule

Stay on O3DE unless the measured evidence shows a real throughput, stability, or performance failure against PeterCraft's concrete needs.
