# Tekken6Jin8 standalone v0.4 — gameplay-first experiment

This package does NOT require PPSSPP Cheats to be enabled. Keep both old
Jin8 Auto Gate and previous Jin8 PRX plugins DISABLED/REMOVED. Do not use
the old 511,607-record auto CWCheat INI from v0.3 alongside this PRX.

Standalone behavior:
- Reads the original binary master data, performs module-range preflight,
  suspends game user_main, applies 511,607 original records ONCE per fight,
  validates beginning/end signatures and resumes user_main.
- Caches 2,458 original Jin runtime records in RAM at startup.
- Applies the cached runtime every ~20ms while Jin P1 is in a fight.
- Never reads 4 MB master data again once installed for that fight.
- Never opens/reads the runtime file during active gameplay.
- Does not modify Tekken6Ultrawide PRX or its manifest.
- Requests expanded RAM (memory = 80) independently.
- Keeps its module-overlap protections and emits Jin8.log.

Installation: back up saves and your Memory Stick. Fully close PPSSPP,
remove old PSP/PLUGINS/Tekken6Jin8/ and Tekken6Jin8Gate/ folders, then
extract this ZIP into your PPSSPP memory stick. It contains:
 PSP/PLUGINS/Tekken6Jin8/plugin.ini
 PSP/PLUGINS/Tekken6Jin8/Tekken6Jin8.prx
 PSP/PLUGINS/Tekken6Jin8/master.j8c
 PSP/PLUGINS/Tekken6Jin8/runtime.j8c
Keep your ultrawide plugin intact; disable Cheats entirely for the first test
(PPSSPP Settings/System). Enable only Jin8 Standalone and ultrawide plugins.
Cold boot, select Jin P1 and enter a match.

If it fails: STOP testing, disable this new plugin and cold-boot. Collect
PSP/PLUGINS/Tekken6Jin8/Jin8.log and PPSSPP crash screen. This is an
experimental compiled build, NOT verified as stable in Android gameplay.
Font/texture replacement compatibility is deferred, per user priority.

If source code refuses memory overlap or cannot suspend user_main, it fails
closed without applying master rather than risk damaging module execution.

## v0.4.1 critical test requirement

**Before another test:** PPSSPP's last crash screen reported `Cheats: Y`. The rejected v0.3 ZIP installed `_C1 Jin8 Auto Master (One-Time)` and `_C1 Jin8 Auto Runtime (After Master)`. **Turn Cheats OFF entirely**, ensure old Jin8Gate PRX is disabled/removed, and cold boot. Running both old auto-cheats and standalone Jin8 can conflict. The guarded v0.4.1 waits for current-move pointer readiness and ~2 seconds of a stable Jin fight state before its master pass. If it crashes, stop and provide `Jin8.log`; do not keep blindly retrying. [Investigation](../research/jin-lifecycle/incident-003-v04-prematch-null-jump.md).

## v0.4.2 — legacy conflict guard / first-runtime logging

The v0.4.1 Android crash screen still showed `Cheats: Y`, and its `Jin8.log` ended after `user_main resumed`. **The new build detects the v0.3 auto-cheat gate or an already populated master and refuses an overlapping initialization**, logging `CONFLICT`. It also logs the start and completion of its first runtime pass. This does **not** establish that the PC=0x10 crash is fixed. For the next and only necessary control, globally disable PPSSPP's Cheats feature (not just individual groups) and remove older Jin8Gate. Cold boot with official ultrawide unchanged and send a fresh log if anything fails. [Incident details](../research/jin-lifecycle/incident-004-v041-master-resume-crash.md).

## v0.4.3 — JIT range coherence and master-only diagnostic

v0.4.2 crashed even after logging `first runtime pass completed`. The separate PRX previously invalidated only the first 2 MiB of guest user RAM, whereas PPSSPP native CWCheat invalidates code caches for writes throughout memory. The new v0.4.3 batches all changed master targets and runtime/memcpy writes and invalidates guest JIT accordingly. **The fix is compiled but not verified on Android**; the official ultrawide PRX/manifest are unchanged and the original data is unchanged.

Turn global PPSSPP Cheats OFF and remove `Tekken6Jin8Gate` before testing. For one optional diagnostic after a crash, create empty `PSP/PLUGINS/Tekken6Jin8/Jin8.masteronly`, cold boot: the master loads but the runtime is skipped. This is not a playable final mode. Remove this marker after testing. See [Incident 005](../research/jin-lifecycle/incident-005-v042-jit-coverage.md).
