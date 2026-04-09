# Engine Adapters

This area owns engine-bound seams for input, save, navigation, audio, UI, and playable scene loading.

- Keep adapter interfaces minimal and stable.
- Keep gameplay rules out of engine-specific implementations.
- `src/` keeps the null/headless backend.
- `o3de/` now keeps the Windows-only Phase 7.1 O3DE bootstrap and playable adapter layer.
