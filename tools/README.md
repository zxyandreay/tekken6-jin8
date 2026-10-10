# Research tools

- `probes/ppsspp-smoke.mjs` — remote debugger transport, version, game ID, and CPU status.
- `probes/capture-jin-state.mjs` — **read-only** capture of bounded Jin-state candidates, aspect instruction words, and memory samples.
- `probes/compare-jin-states.mjs` — offline JSON comparison of captures, no emulator connection.
- `cheat_inventory.py` — optional offline inventory/hash of your original CWCheat file; does not reconstruct or redistribute it.

Node.js 22+ is recommended. No npm install or PPSSPP binary modification is required.

- Add `--inspect-pointers` to `capture-jin-state.mjs` to read at most 384 bytes from the two validated guest pointer candidates (no memory writes, no recursive pointer chase).
