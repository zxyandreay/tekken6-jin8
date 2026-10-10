# PPSSPP remote debugging

**Status: FIRST PROBES READY; live capture not yet performed.**

These instructions follow [PPSSPP's developer tools](https://www.ppsspp.org/docs/development/developer-tools/) and [WebSocket debugger source](https://github.com/hrydgard/ppsspp/blob/master/Core/Debugger/WebSocket.cpp), plus methodology from [tekken6-ultrawide](https://github.com/zxyandreay/tekken6-ultrawide).

## Installation and connection

- Install Node.js **22 or later** on the computer running probes (built-in `WebSocket`; no npm install).
- In PPSSPP: **Settings → Tools → Developer Tools → Allow remote debugger**.
- Run Tekken 6 USA, `ULUS10466`. Read the debugger host/port from PPSSPP's output/log; do **not** assume port 39100.
- If probes run beside PPSSPP, pass `--endpoint 127.0.0.1:<port>`; if from another device, use its LAN IP. Don't make the debugger reachable from the public internet.
- The WebSocket path is `/debugger`, subprotocol `debugger.ppsspp.org`.

```powershell
node tools/probes/ppsspp-smoke.mjs --endpoint 127.0.0.1:39100
node tools/probes/capture-jin-state.mjs --endpoint 127.0.0.1:39100 --label stock-menu
node tools/probes/capture-jin-state.mjs --endpoint 127.0.0.1:39100 --label stock-jin-battle
node tools/probes/compare-jin-states.mjs captures/stock-menu.json captures/stock-jin-battle.json
```

`39100` is **an example**. If these fail, first check the actual port and WebSocket upgrade, then game ID and CPU status. Don't misclassify a connection error as a memory finding.

## Probe safety

- The included probes make only requests: `version`, `game.status`, `cpu.status`, `hle.module.list`, `memory.read_u32`, and `memory.read`.
- No `memory.write*`, `cpu.breakpoint.*`, `cpu.resume`, `cpu.stepping`, or load/install actions.
- Snapshot errors for invalid addresses are logged per address. No pointer chasing.
- Output under `captures/` is ignored by git and must be inspected before sharing.
- Pause the game **manually** when you need a perfectly consistent snapshot (especially when comparing mid-move states). These probes do not pause PPSSPP.

## Capture sequence

Follow [first Jin lifecycle experiment](../../research/jin-lifecycle/README.md). Start with a **clean game**: disable the new Jin mod/master cheat and ultrawide plugin. Cold boot between distinct baselines.

Save individual snapshots with distinct labels, e.g. `stock-menu`, `stock-jin-select`, `stock-jin-battle`, `mod-jin-battle`. Capture phase info and emulator version in notes when you report results.

For comparisons, the helper prints changes to memory probes and ultrawide instruction sites; changes alone do not establish their meaning.

## Next debugger stage

After baseline snapshots verify stable pointers, write **targeted and bounded** breakpoint probes for moveset load and current-move transitions. Instrumentation will be separate from the read-only first probe. Every breakpoint script must remove its own breakpoints and resume any CPU state it owns on error/timeout.

## Troubleshooting

- `ECONNREFUSED`: debugger server disabled or wrong port/host.
- Upgrade/subprotocol failure: wrong port or URL scheme/path.
- Wrong game ID: launch Tekken 6 USA, not another region.
- Invalid address: game hasn't allocated that state yet or memory layout differs.
- No useful diff: take snapshots in the specified phases, and make sure the game is paused manually for fine-grained comparisons.

## Deep pointer-window follow-up

Experiment 001 identified two nonzero aligned candidate pointer values in an active stock Jin fight. The capture script now accepts `--inspect-pointers`: it performs no more than two additional bounded `memory.read` requests (256 bytes at the candidate moveset pointer and 128 bytes at the candidate current-move pointer). It validates alignment and guest-memory bounds first and will log read errors instead of forcing access.

```powershell
node tools/probes/capture-jin-state.mjs --endpoint 192.168.0.101:39100 --label stock-jin-battle-deep --inspect-pointers
# Separate cold boot: use the original known-working mod activation before taking this:
node tools/probes/capture-jin-state.mjs --endpoint 192.168.0.101:39100 --label mod-jin-battle-deep --inspect-pointers
node tools/probes/compare-jin-states.mjs captures/stock-jin-battle-deep.json captures/mod-jin-battle-deep.json
```

The debugger endpoint is from the user's previous captures; adjust if it changes. These commands do not enable cheats or pause the CPU. When possible, pause manually in PPSSPP's debugger **after** entering the target fight before capturing; the returned CPU status will record whether it was paused. Move pointers may change between requests while running.

Inspect the JSON under ignored `captures/` before sharing. The differences are descriptive and do not automatically identify pointer ownership, correct structure layout, or successful master activation.
