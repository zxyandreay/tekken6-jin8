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
