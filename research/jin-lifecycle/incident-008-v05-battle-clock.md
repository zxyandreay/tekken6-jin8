# Incident 008 / Experiment 007 — v0.5 battle-clock-triggered standalone master

**Status:** Compiled experimental build; gate unit tests and PSPDEV build passed. **No Android gameplay validation yet.** This is not declared a confirmed fix for the repeated post-master-resume PC=0x10 crash.

## Confirmed prior failure (source: user and log)

- Android PPSSPP v1.20.4, Tekken 6 USA ULUS10466, Cheats **OFF**, `Jin8.masteronly` present: bad execution address `PC=0x00000010`, `RA=0x088E78EC`.
- Exact `Jin8.log` confirms `MASTER-ONLY diagnostic enabled`, master preflight passed, `user_main suspended`, JIT invalidation completed, `master initialized (2 signatures)`, then `user_main resumed`. No Jin runtime execution in this mode.
- Therefore leftover native CWCheats and the small Jin runtime are **not necessary** for this crash. Something about the standalone master/application path or effects remains unsafe.
- Previous versions v0.4.1+ waited 2 seconds after valid Jin character ID, moveset pointer, current move pointer and move ID. These values can become populated in character/stage loading, so they are an **insufficient proxy** for actual battle gameplay.

## New v0.5 implementation: verify countdown progression before any master write

- Independent source: `plugin-standalone/src/battle_clock.h` plus `plugin-standalone/src/main.c`.
- Tekken 6 PSP community cheat references use `_L 0x208FBE4C 0x00000E11` to freeze the round timer at 60 seconds. This corresponds to **guest `0x090FBE4C`** (CWCheat base `0x08800000`). Source: [Tekken 6 PSP cheat codes](https://id.scribd.com/document/102814346/Tekken-6).
- An offline scan of every `(m)Tekken 8` record from the exact user-owned `ULUS10466 (3).ini` verified **zero direct master writes** target `0x090FBE4C`. This makes the timer a plausible independent gameplay signal.
- The plugin still validates Jin ID=9, live moveset base variant, current move pointer, move ID, and a continuous ~2s stable Jin base.
- **NEW:** Once pointer state is stable, the plugin checks the round timer every 25ms. It requires an in-range value `1..0x3000`, **8 plausible consecutive decreases** (each decrease 1..12 units), then 20 further decreasing observations. Value resets, excessive jumps, invalid values, or prolonged steady values reset/reject detection.
- **Fail-closed:** If timer is constant, unsupported (e.g. non-countdown Practice mode), or not at the anticipated address for this fight mode, the plugin does **not** initialize the master. This can result in stable default Jin with no mod; log says `waiting for active round timer (no writes yet)`. This is preferable to the confirmed deterministic crash before the fight.
- Upon confirmed timer movement the original preflight, module protection, original 511,607-record master, deferred cache synchronization, and cached 2,458-record runtime proceed unchanged. It is still a standalone PRX requiring **NO PPSSPP Cheats**.
- Official `zxyandreay/tekken6-ultrawide` source, release, PRX and manifest are not modified. Master binary resources remain user-provided, unchanged and excluded from public Git history.

## Validation

- New deterministic host C test `tests/battle_clock_test.c` exercises ordinary decreasing 60s timer, a frozen timer, invalid/out-of-range timer, and round reset. The test is executed in a separate Ubuntu GitHub Actions job (the PSPDEV container does not include host `gcc`).
- PSPSDK build: [run 38046683524](https://github.com/zxyandreay/tekken6-jin8/actions/runs/38046683524), **success**, including separate host `battle-clock-host-test` job.
- Repository research checks: [run 38046683487](https://github.com/zxyandreay/tekken6-jin8/actions/runs/38046683487), **success**.
- New PRX `Tekken6Jin8.prx`: 39,418 bytes, mapped ELF resident `0x8394` (33,684 bytes), SHA-256 `23494febb36161148ab45b396d3e86ed55fa9d6248a220d9485238fa22704a2d`.
- Original `master.j8c`: 511,607 records, SHA-256 `956b045210749885ad21da15c0c190f5d6db34fdd8fb998c517c5097fb4c37ca`.
- Original `runtime.j8c`: 2,458 records, SHA-256 `ace847db19328eca86b05412d9ee8f4cc1349ec34ea97f0630c7f570dc7a68aa`.
- User-specific v0.5 ZIP: `Tekken6Jin8-v0.5-BattleClock-Standalone-NoCheats-ULUS10466.zip`, **2,426,718 bytes**, SHA-256 `dc6fb5fd061d15adfbbf10685f355fa524899e7300cb0236444cac88137aa203`. Combines freshly built PRX/manifest with unchanged supplied data.
- Compiled ZIP is installable and structurally validated, **not tested in an Android game session**.

## Installation/testing limits and next direction

1. **Remove both older Jin8 PRXs**, including the Jin8 Gate, and remove any lingering `Jin8.masteronly` marker. Fully close PPSSPP and install the v0.5 package in `PSP/PLUGINS/Tekken6Jin8`.
2. Keep PPSSPP global Cheats **OFF**, original official ultrawide enabled/untouched, and perform one cold boot of Tekken 6 USA Jin P1.
3. When the normal round starts, the timer should begin decreasing. The plugin waits for gameplay timing confirmation, then attempts automatic master setup. Record the log stage and whether Jin moves work and remain smooth.
4. If battle plays as ordinary Jin and the log stays at `waiting for active round timer`, **do not assume the move loader executed**. The round timer signal may not hold in this game mode; next step is identify a supported in-fight signal without manually enabling Cheats.
5. If crash persists, stop testing and inspect new `Jin8.log` stage; do not repeat failed master-only tests blindly. A later crash still needs genuine master engine semantic parity or state analysis, not arbitrarily extended delays.
6. HD custom-font/texture replacement remains lower priority by user request. PRX footprint exceeds previously validated ultra-small plugin sizes.

## Historical corrections

- Do **not** label prior compiled iterations stable: v0.1/v0.2 native interpreter crashed, v0.3 native CWCheat gate partly worked but stuttered and violated no-Cheats design, and v0.4–v0.4.3 crashed after master resume.
- The v0.5 clock signal is supported by an external Tekken 6 timer cheat and a no-overlap static scan, but is **not yet verified** with on-device runtime memory capture for this fight mode. The accepted next build must demonstrate both safe battle entry and correct Jin8 moves.
