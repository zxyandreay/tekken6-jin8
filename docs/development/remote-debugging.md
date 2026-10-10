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

## Camera/slowmo diagnostic — Investigation 002

The repacked `(1)jin` source itself includes camera zoom and SlowMo sections; unusual camera effects need not come from the ultrawide PRX. The remote debugger now records four **read-only candidate effect-control words**, as well as the shared camera instruction:

| PSP address | Originating source-cheat operand | Working hypothesis |
| --- | --- | --- |
| `0x08C52890` | `0x20452890` | Game-speed controller candidate |
| `0x08D6F270` | `0x2056F270` | Camera-speed controller candidate |
| `0x08D6F224` | `0x2056F224` | Camera zoom controller candidate |
| `0x08C07CA0` | `0x20407CA0` | SlowMo timer/state candidate |

These interpretations are **inferred from source labels and published notes**, not yet tested in the user's game. They are passive reads, not freeze/write cheats. Capture unchanged camera settings in stable fights vs mid-effect if the game is playable. If PPSSPP becomes unstable, stop testing and cold boot without the cheats rather than trying to restore RAM by unchecking the box.

The original ZIP was not a verified complete distribution. See [Investigation 002](../../research/cheat-format/investigation-002-effects.md). For clean isolation, disable the ultrawide plugin **only if you can still satisfy the source payload's memory requirement**; otherwise run a labeled combined-mod diagnostic with optional camera CWCheats disabled.

## Experiment 002: sampled master signatures

After first/late source writes both matched user captures, `verify-master-payload.mjs` was added to test **11 distributed known 32-bit direct writes** from the exact source master (no user-supplied cheat file needed on the probe computer). Run with cheats OFF and then in a separate successfully initialized Jin fight with the master toggled OFF and Jin runtime ON; keep the **same ultrawide/expanded-memory setting** for both phases:

```powershell
node tools/probes/verify-master-payload.mjs --endpoint 192.168.0.101:39100 --label master-stock-matched
node tools/probes/verify-master-payload.mjs --endpoint 192.168.0.101:39100 --label master-mod-matched
```

Also take a new `--inspect-pointers` stock capture **with ultrawide already enabled**, because the first deep comparison had ultrawide **OFF only in stock**. Disabled camera cheats are preferred in both sessions. Use a fresh cold boot for each phase; while the mod master is executing, avoid other memory probes until initialization is complete. The signature probe only reads 11 words and writes JSON locally into ignored `captures/`. It does not activate cheats, install hooks or change emulator state.

Results with 11/11 matching signatures would support broad payload presence but **cannot prove all 511,607 master records were installed or runtime conditions work**. See [Experiment 002 report](../../research/jin-lifecycle/experiment-002-results.md).

## Experiment 003: passive conditional runtime pointer probes

The master payload is now confirmed at all **11 of 11** sampled writes with ultrawide present in both stock and mod tests. Further master-sample replays have diminishing value. The more important unresolved question is the source runtime's conditional **0x6 pointer writes** into deep offsets of the move table (source examples: `0x604078E0 0x0909CE00` then `0x00020001 0x0002C250`).

A new `trace-jin-runtime-links.mjs` probe follows **one validated pointer** at `0x08C078E0` and reads eight explicitly known offsets (no recursive chasing or writes). These are **conditional** values, so `0 matches` during idle is **not a load failure**.

```powershell
node tools/probes/trace-jin-runtime-links.mjs --endpoint 192.168.0.101:39100 --label links-stock-idle
# Cold-boot same plugin/memory setup; manually initialize Jin master, then return to normal Jin idle:
node tools/probes/trace-jin-runtime-links.mjs --endpoint 192.168.0.101:39100 --label links-mod-idle
# Optional if the mod is stable and a move reliably triggers the effect:
node tools/probes/trace-jin-runtime-links.mjs --endpoint 192.168.0.101:39100 --label links-mod-attack --samples 20 --interval-ms 200
```

The CLI stores distinct JSON under ignored `captures/`. Never reuse a label already present, since output uses non-overwriting `wx`. Record which move/input was performed and whether odd camera or SlowMo behavior occurred.

**Camera probe correction:** `shared_camera_word` at `0x0895350C` is executable code; the snapshot tool now reads it via `memory.read` `replacements:false`, not `memory.read_u32`. The old stock read `0x682CE4C8` may be a JIT/emuhack replacement rather than an actual camera patch. This interpretation is pending a raw-code capture. Do not use that prior sample to claim a bug in ultrawide.

Also see [Experiment 003 findings](../../research/jin-lifecycle/experiment-003-results.md).

## Experiment 004: watch individual move conditions

After observing three static custom pointer links with `trace-jin-runtime-links.mjs`, use the new **passive** `watch-jin-move-conditions.mjs` to correlate move IDs, input state, pointer-base changes, effect controllers, and conditional pointer destinations. It re-resolves the active table base each sampling cycle and flags changes during sequential reads.

Run once in the already initialized modded Jin fight (no further master toggles):

```powershell
node tools/probes/watch-jin-move-conditions.mjs --endpoint 192.168.0.101:39100 --label jin8-move-one --samples 40 --interval-ms 200
```

Perform **one** reproducible move several times after the probe starts; note the input and whether zoom, slowdown, or a Tekken 8-looking animation occurs. The source specifically gates link commands on current move IDs `0x0266`, `0x043D`, and `0x02E3`; the source labels `0x043D` next to `// 1+2+3+4`. This is not proof the same input works in the current game state. Use a new label for a second move (e.g. `jin8-move-two`). The code never writes, freezes, or pauses gameplay. Its reads are sequential and may miss short-lived moves; a nonmatch is not conclusive.

[Full Experiment 004 source comparison](../../research/jin-lifecycle/experiment-004-results.md).

## Experiment 005: focused cinematic event monitor

The earlier move watcher took roughly **0.5–0.6 s per complete sample** despite a `--interval-ms 200` setting. One attack capture nonetheless observed an exact speed/zoom/camera-rate combination found in the original `(1)jin` SlowMo source. The user reported P2 attacks during idle and could not deliberately reproduce slow-motion. See [Experiment 005](../../research/jin-lifecycle/experiment-005-results.md).

For the next attempt prefer **Practice**, with P2 set to **stand still/do nothing**, rather than an attacking CPU. Avoid assuming a saved `slowmo` filename proves an event occurred. Once modded Jin is initialized, run a limited read-only control and one known deliberate attack:

```powershell
node tools/probes/watch-jin-cinematics.mjs --endpoint 192.168.0.101:39100 --label jin8-practice-no-input --samples 60 --interval-ms 100
node tools/probes/watch-jin-cinematics.mjs --endpoint 192.168.0.101:39100 --label jin8-practice-one-attack --samples 80 --interval-ms 100 --after-detection 8
```

If a trigger is seen, the monitor captures post-event samples and exits; otherwise it uses the whole requested bounded window. It reads nine fixed known addresses only; no writes, breakpoints, or CPU changes. It flags **speed mode 3 or zoom below 40** as *diagnostic candidates*, not proof of visibly slowed gameplay. Keep the opponent and input conditions documented. No need to deliberately recreate slow-motion repeatedly or re-run the huge master to collect a triggered event.
