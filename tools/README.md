# Research tools

- `probes/ppsspp-smoke.mjs` — remote debugger transport, version, game ID, and CPU status.
- `probes/capture-jin-state.mjs` — **read-only** capture of bounded Jin-state candidates, aspect instruction words, and memory samples.
- `probes/compare-jin-states.mjs` — offline JSON comparison of captures, no emulator connection.
- `cheat_inventory.py` — optional offline inventory/hash of your original CWCheat file; does not reconstruct or redistribute it.

Node.js 22+ is recommended. No npm install or PPSSPP binary modification is required.

- Add `--inspect-pointers` to `capture-jin-state.mjs` to read at most 384 bytes from the two validated guest pointer candidates (no memory writes, no recursive pointer chase).

- `probes/verify-master-payload.mjs` — bounded read-only 11-anchor checksum probe across the user's original master (no mod data embedded). Use the same plugin/memory configuration for stock vs active-mod comparisons.

- `probes/trace-jin-runtime-links.mjs` — read-only sampler of eight source-derived, conditional 0x6 pointer-write destination addresses; optional bounded repeat sampling (max 20 reads per address). No writes or breakpoints.

- `probes/watch-jin-move-conditions.mjs` — bounded read-only move/event watcher (live moveset pointer resolution, move IDs, selected conditional links, camera/speed field samples). Max 80 snapshots; samples are sequential, not atomic.

- `probes/watch-jin-cinematics.mjs` — focused, read-only nine-address monitor of speed mode, zoom, camera rate, move IDs, and game-effect gates. Auto-flags interesting state and saves a bounded event tail. No installs and no mod edits.
