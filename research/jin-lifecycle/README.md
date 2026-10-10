# Experiment 001 — Jin moveset lifecycle, read-only

**Status: READY FOR USER CAPTURE. No local PPSSPP results recorded yet.**

## Question

When do Jin's character and moveset pointers become valid, when are they replaced, and what changes after the original master activation?

## Minimum A/B matrix

Run the snapshot tool at the specified moment (pause PPSSPP manually first for consistency). **Do not apply the huge master cheat at character select; preserve the known working procedure when testing it.**

| Label | Setup | Capture point |
| --- | --- | --- |
| `stock-menu` | Jin cheat and ultrawide both disabled; cold boot | Main menu |
| `stock-jin-select` | Same stock session | Jin highlighted on character select |
| `stock-jin-battle` | Same stock session | Jin P1, active fight, no cheat |
| `mod-jin-battle` | New clean boot with original cheat; follow its known enable-then-disable master timing | Jin P1, active fight, after manual master is disabled, runtime Jin cheat stays active |
| `ultrawide-jin-battle` | Separate clean boot, only accepted ultrawide PRX and desired camera cheat | Jin P1, active fight |
| `combined-jin-battle` | Separate clean boot, both known mods under existing manual procedure | Jin P1, active fight |

**Do not run the full matrix until the smoke test succeeds.** The minimum useful next response is smoke output plus `stock-menu` and `stock-jin-battle` captures.

## Example commands

```powershell
node tools/probes/ppsspp-smoke.mjs --endpoint 127.0.0.1:39100
node tools/probes/capture-jin-state.mjs --endpoint 127.0.0.1:39100 --label stock-menu
node tools/probes/capture-jin-state.mjs --endpoint 127.0.0.1:39100 --label stock-jin-select
node tools/probes/capture-jin-state.mjs --endpoint 127.0.0.1:39100 --label stock-jin-battle
# For the mod state, only after a fresh boot and after running the known manual method:
node tools/probes/capture-jin-state.mjs --endpoint 127.0.0.1:39100 --label mod-jin-battle
node tools/probes/compare-jin-states.mjs captures/stock-jin-battle.json captures/mod-jin-battle.json
```

Replace `39100` with the actual PPSSPP remote debugger port. The script is passive: **no hook, plugin, cheat, or breakpoint is installed**.

## What to report back

- PPSSPP version, game region, CPU backend (JIT/interpreter), expanded-memory setting, and whether each plugin was enabled.
- Smoke output and first capture files (or summary JSON), with exact label/moment and any read errors.
- Whether Jin uses his stock moveset or the modded moveset in each fight; whether Rage Art can activate.
- Screenshots or short clips of successful activation if available.

## Interpretation policy

Do not declare a pointer address proven by a single non-zero value. Compare multiple captured phases, stable repeated runs, other characters, and next-match reloads. If full mod cannot activate reliably, record failed steps and collect mod-disabled controls first.

## Baseline completed

The first `stock-menu` and `stock-jin-battle` captures are analyzed in [Experiment 001 results](experiment-001-results.md). Mod-enabled and pointer-target captures are still pending. Do not interpret nonzero pointer words as a confirmed move-table hook.

## Second-stage bounded pointer capture

Follow [remote debugging instructions](../../docs/development/remote-debugging.md#deep-pointer-window-follow-up). Re-capture stock and successfully modded Jin in the same match phase using `--inspect-pointers`, then compare. This is passive; the source mod must still be activated manually for the mod-side baseline.

**Control caveat:** Both first captures contain a camera word matching the ultrawide optional `Camera - Wider` setting. Repeat with camera cheats disabled if a completely stock camera control is needed; otherwise keep camera conditions identical across A/B and record the setting.

## Experiment 002 checkpoint

[Deep capture findings](experiment-002-results.md): the master starts and late source-write signatures **both match live memory**; however, the stock and mod configurations differed (ultrawide absent vs present), and CPUs were not paused. Run matched captures and the new read-only signature probe rather than concluding all writes completed.

```powershell
node tools/probes/verify-master-payload.mjs --endpoint 192.168.0.101:39100 --label master-stock-matched
# after cold boot and deliberate manual activation in a stable Jin fight:
node tools/probes/verify-master-payload.mjs --endpoint 192.168.0.101:39100 --label master-mod-matched
```

## Experiment 003 (matched setup)

[Findings](experiment-003-results.md): With ultrawide and expanded memory in both states, **0/11 stock vs 11/11 mod** signatures. Next compare runtime pointer targets with `trace-jin-runtime-links.mjs` and capture real known move triggers. The camera read now bypasses PPSSPP instruction replacements.

## Experiment 004 checkpoint

[Runtime pointer result](experiment-004-results.md): 3/8 source-defined custom links match after the mod, 20/20 stable for those three in attack sampling, 0/8 stock. Both `0x086A` and `0x0875` moveset prefixes are explicitly handled by the cheat. The next stage samples move ID and conditional link together with the new read-only watcher.

## Experiment 005 checkpoint

[Three watcher captures analyzed](experiment-005-results.md). P2 was attacking during nominal idle, so it is not a stationary control. Three custom links persist across 100/100 samples. A genuine source-matching **speed mode 3 / zoom 16.0 / camera rate 0.5** window appeared in the attack capture, despite the user not perceiving SlowMo. Next reduce P2 interference and test the read-only focused cinematic monitor.

## Android incidents and automatic implementation

v0.1 crashed and altered texture-font replacement selection; v0.2 reduced resident footprint but still crashed **at Jin fight start** while menu fonts remained wrong. See [Incident 001](incident-001-android-crash-fonts.md) and [Incident 002 + lite v0.3 pivot](incident-002-v02-crash-lite-v03.md). Follow-up is **testing the new implementation** rather than repeating the original source-cheat runtime probes.

## Experiment 006 / gameplay-first standalone pivot

User observed partially working Jin8 animations but repeated stutters under v0.3; explicitly rejected any implementation requiring Cheats enabled. [v0.4 standalone optimization report](experiment-006-v04-standalone-optimization.md): cache runtime once, master only once under protected fight transition; no PPSSPP Cheats dependence. Font regressions deferred. The new PRX builds but requires an Android test before being considered stable.

## Incident 003: v0.4 before-battle crash

[Crash report and guarded v0.4.1](incident-003-v04-prematch-null-jump.md): PC `0x10`, RA `0x0893F4D8`, possible leftover auto-CWCheat conflict plus insufficient fight-load readiness. Needs Cheats OFF, v0.3 gate removed, cold boot, and `Jin8.log`. Existing source/master files remain unchanged.

## Incident 004 — master completed, game immediately crashes

[Incident 004](incident-004-v041-master-resume-crash.md): attached v0.4.1 Jin8.log proves master initialized two signatures and user_main resumed, then the crash screen reports PC=0x10/RA=0x0889691C. Master opcode audit and stale v0.3 cheat conflict documented. Compiled v0.4.2 guards duplicate initialization and records first runtime boundary. **No Android stability evidence yet.**

## Incident 005: incomplete JIT write coverage in custom interpreter

[Incident 005](incident-005-v042-jit-coverage.md): user log shows master and first runtime completed in v0.4.2, then PC `0x10` crash; host CWCheat comparison found the PRX failed to invalidate guest JIT outside the first 2 MiB and across memcopies. v0.4.3 batches all-range writes. Android result unknown.
