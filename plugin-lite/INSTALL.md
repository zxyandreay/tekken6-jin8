# Jin8 Lite Auto v0.3 — installation and test

**Requirements:** Tekken 6 USA `ULUS10466`, PPSSPP with **Plugins + Cheats enabled**.

The v0.3 design consists of **two pieces**:

1. `PSP/PLUGINS/Tekken6Jin8Gate/plugin.ini` and `Tekken6Jin8Gate.prx`, independently compiled.
2. `PSP/Cheats/ULUS10466.ini`, generated using `tools/build_auto_cheats.py` from the specific original user-supplied 13MB cheat file; the original third-party cheat records are never committed publicly.

**The PRX itself does not implement moves or write into game code**, and is intended to coexist with the unmodified official Tekken6Ultrawide PRX.

## Installation

1. Back up saves and the shared `PSP/Cheats/ULUS10466.ini`.
2. Disable and remove the **old** rejected `PSP/PLUGINS/Tekken6Jin8/` v0.1/v0.2 loader. Do not leave it running beside Lite.
3. Copy new gate PRX/manifest under the distinct `PSP/PLUGINS/Tekken6Jin8Gate/`.
4. Replace/merge the generated `ULUS10466.ini` in `PSP/Cheats/`. Generated file keeps the official ultrawide camera presets (default OFF), but **does not preserve unknown unrelated personal cheats**: carefully merge those after backing up. This step is a shared PPSSPP cheat-data change, **not** an official ultrawide plugin modification.
5. Enable PPSSPP Plugins and Cheats. Enable `Tekken 6 Jin8 Auto (Experimental v0.3)`. Verify auto master and auto runtime groups are ON (marked `_C1`), without the original manual groups also running. Leave camera preset cheat groups OFF for the first test.
6. Completely restart PPSSPP; launch a **new** Tekken 6 session, not an old save state.
7. Check your custom HD font/text textures **on the main menu**, before selecting Jin.
8. Select Jin P1 and start a fight. The tiny gate detects the loaded Jin moveset and permits **one scheduled native CWCheat master pass**. On completion, the native runtime group automatically becomes active. The first frame can stall while PPSSPP processes the 511k master records.
9. Confirm Jin's custom attacks and original ultrawide aspect/HUD/texture behavior. This is a first experimental device check, not a validated release.

## Why it is safer than v0.2

The old loader's large PRX changed memory allocation and wrote asynchronously from its own PSP thread, likely exposing partially initialized game pointer data. This lite PRX maps only 1,076 bytes and writes **only one marker** (`0x0A1FFFC0`). The original source writes happen in PPSSPP's *existing* CWCheat scheduler, with conditional gates to prevent premature and repeated installation. The Android results are still unknown.

## Recovery

Disable/remove `Tekken6Jin8Gate` and restore the backed-up `ULUS10466.ini`; then cold-boot. Keep the original ultrawide plugin PRX and manifest unchanged.

If menu fonts are already missing before Jin loads, report that specifically; it could indicate a remaining module-allocation or native-CWCheat mapping effect rather than an injected moveset change. If the game crashes during the Jin fight again, stop and send the crash screen and whether the main-menu HD font textures were active.
