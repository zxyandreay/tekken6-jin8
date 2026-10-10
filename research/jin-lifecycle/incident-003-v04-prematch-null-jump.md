# Incident 003 — v0.4 before-battle null execution jump (2026-10-10)

**User report:** Android PPSSPP v1.20.4 (release), Xiaomi `23049PCD8G`, game `ULUS10466`, known-good CRC `ae9abfb0`.

## Direct observations from attached screenshot and report

- Main menu and Jin character selection have **no issues**.
- Crash occurs **before the fight/battle loads**, not after established playable match.
- PPSSPP purple screen: `Bad Execution Address`, `Destination / PC = 0x00000010`, `RA = 0x0893F4D8`, thread `user_main`, CPU JIT.
- Screenshot shows `Cheats: Y; Plugins: Y`, **which reports features enabled, not a list of active CWCheat groups**.
- The v0.3 user-specific ZIP was independently rechecked: it installed `PSP/Cheats/ULUS10466.ini` containing *two enabled-by-default* `_C1` entries: `Jin8 Auto Master (One-Time)` and `Jin8 Auto Runtime (After Master)`.
- If that INI remained installed and Cheats was still enabled, **v0.3's native master/runtime could run simultaneously with v0.4's standalone interpreter**. This is a serious unresolved confound. It is **not confirmed** that both groups were actually executing on the user's device.
- No `Jin8.log`, active-cheat list, debugger register dump, or crash stack beyond the screenshot was provided. Do **not** claim the exact invalid-jump cause is proven.

## Additional offline EBOOT instruction check

Original decrypted `ULUS10466_EBOOT(6).BIN` is ELF32 little-endian MIPS, text loaded at guest `0x08804018`. Instruction at `RA - 8 = 0x0893F4D0` is `0x0E245C42` = a standard MIPS **JAL to `0x08917108`**. That callee contains `LUI v1, 0x08C1; JR ra; LW v0, 0x40B4(v1)`, reading the game's state from **`0x08C140B4`**. The original Jin runtime source contains *conditional reads* of that address but no direct `_L` write to it in the checked source. The return-address context establishes the call site but **does not establish whether a master write, JIT mismatch, null pointer or an overlapping enabled cheat caused the branch to `0x10`**.

## Actual v0.4 engineering defect/risk

Previous standalone `jin_fight_ready()` checked only P1 Jin ID, a moveset base with a recognized high-word and a nonzero move ID. These values can potentially become nonzero before the entire stage, renderer, and move tables have finished initializing. The source cheat was historically manually enabled *after entering a fight*. The PRX had no battle-stability settle gate, so it might initialize too early. This is **a concrete insufficient readiness check**, but its relationship to the reported crash remains a hypothesis.

The v0.4 engine also has code-level differences from PPSSPP's reference `Core/CwCheat.cpp`, so a genuine interpreter bug cannot be ruled out if the crash persists after a clean-cheat test. Do not attribute all crashes to timing.

## v0.4.1 guarded implementation

- Independent standalone `plugin-standalone/src/main.c`. PPSSPP Cheats is **not needed**.
- Require valid current move pointer at `[0x08C07964]`, aligned and in the observed guest-game move-record window, as well as a nonzero current move ID, Jin ID and recognized moveset base.
- Require a **continuous ~2-second window** (80 cycles of 25ms) with stable moveset base and valid per-sample current move before applying master. If condition drops or base changes, reset the window. The game should be in a more stable battle-ready state before source writes.
- Preserve `master.j8c` and `runtime.j8c` byte-for-byte from original source; cache runtime and apply master once, same as v0.4. The additional delay is a **risk reduction, not proof of correct battle-ready timing**.
- Preserve unmodified official `Tekken6Ultrawide` binary, manifest and release.
- Update manifest display/version to **v0.4.1**. Logging includes `Jin move table candidate found; waiting for battle stability` and `Jin battle stable for 2 seconds; checking master targets`.

## Build and artifact provenance

- Source commit: `3d9d8ce0573d722461360dfef45ac7297ee7e3a6`.
- PSPSDK build: [run 38035283253](https://github.com/zxyandreay/tekken6-jin8/actions/runs/38035283253) — **success**.
- PRX file SHA-256: `3cf570472e74c8f547c41d30827ef741ea87be016126d25a13a7c1028e1d5ed7`; PRX ELF32 mapped-memory size `0x77C0` (30,656 B).
- Original master resource hash: `956b045210749885ad21da15c0c190f5d6db34fdd8fb998c517c5097fb4c37ca` (511,607 records).
- Original runtime resource hash: `ace847db19328eca86b05412d9ee8f4cc1349ec34ea97f0630c7f570dc7a68aa` (2,458 records).
- User-specific ZIP `Tekken6Jin8-v0.4.1-Guarded-NoCheats-ULUS10466.zip`: 2,424,229 bytes, SHA-256 `eb585926c6f93495413d0e611f93ff65bdac3dbb0bcb3ce7ab526bf628bb6ba4`.
- Converted mod binary payload remains excluded from public Git repo (unverified third-party redistribution license). Public PRX code and research are committed.

## Mandatory next test and acceptance policy

1. Back up PPSSPP memory stick and game saves. **Turn PPSSPP Cheats completely OFF**. Restore pre-v0.3 cheat file or explicitly disable the two auto groups, and disable/delete old `Tekken6Jin8Gate`. Do not run any older Jin8 plugins simultaneously.
2. Cold-boot once with official ultrawide unchanged and v0.4.1 standalone enabled. Observe whether the menu/fonts, fight, and custom moves work. Initial ~2s guard is expected and not representative of gameplay stutter.
3. If crash occurs, **stop testing**. Recover `PSP/PLUGINS/Tekken6Jin8/Jin8.log` and crash screenshot. The last log stage determines whether failure is before, during or after master pass.
4. If no log exists, first test `Jin8.safe` (empty marker file, cold boot) to isolate module load from any Jin memory writes.
5. Still-crashing isolated v0.4.1 does **not** justify claiming success; investigate interpreter parity against PPSSPP and JIT invalidation behavior before another gameplay build.

**Status: test-ready, compilation verified; no Android run yet. Do not mark the crash solved.**
