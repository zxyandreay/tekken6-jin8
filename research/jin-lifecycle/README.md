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
