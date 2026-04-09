# ADR 0007: Playable Runtime Baseline

## Status

Accepted

## Context

Phase 7.0 established explicit runtime modes and kept the headless shell green, but the playable path was still a stub. Phase 7.1 needs a real engine-backed host without sacrificing the portable gameplay core, deterministic tests, or save/creator safety work already in the repo.

## Decision

PeterCraft will use O3DE `25.10.2` as the active playable runtime baseline in Phase 7.1.

The repo now carries:

- an in-repo O3DE project under `game/o3de/`
- O3DE-specific adapter implementation code under `engine-adapters/o3de/`
- a Windows-only playable bootstrap path behind `--runtime playable`
- portable scene bindings in `game/data/content/scene-bindings/` that map logical scenes to O3DE levels

## Rules

1. Headless remains the default and authoritative path for tests, validation, deterministic scenarios, and CI.
2. Portable gameplay rules stay outside engine-only scripts and asset formats unless there is a concrete runtime need.
3. O3DE integration happens by composition through adapter seams, not by sprinkling engine calls through the core modules.
4. The O3DE baseline is pinned to one exact engine version in Phase 7.1: `25.10.2`.
5. Unreal stays documented as a fallback, but it does not become active unless measured O3DE gates fail.

## Consequences

Positive:

- PeterCraft now has a real engine-backed host path without discarding the portable shell
- contributors have explicit build and run commands for playable work
- logical scene intent remains portable while levels live in the engine project

Tradeoffs:

- Windows is the only supported playable platform in Phase 7.1
- local O3DE installation is now a real prerequisite for playable work
- the adapter layer has to carry bootstrap, launcher, and scene-transition plumbing until deeper real-time systems land
