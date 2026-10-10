# Incident 004 — v0.4.1 crash after master resume; v0.4.2 duplicate-write guard

**User report:** October 10, 2026. Android Xiaomi `23049PCD8G`, arm64, PPSSPP v1.20.4, TEKKEN 6 USA `ULUS10466` CRC `ae9abfb0`.

## Direct user evidence

- Game reaches menus and Jin selection but crashes when the battle/fight loads.
- Purple PPSSPP error: **Bad Execution Address**; `Destination/PC = 0x00000010`, `RA = 0x0889691C`; thread `user_main`, JIT.
- Crash report lists `Cheats: Y; Plugins: Y`. **This does not list which cheats ran**. It nevertheless leaves a serious confound: v0.3 installed two auto-enabled `_C1` master/runtime groups into `ULUS10466.ini`, and the user has not yet explicitly confirmed that the global Cheats feature and those groups were disabled during v0.4/v0.4.1 tests.
- User supplied exact `Jin8.log` with nine lines:

```text
Tekken6Jin8 Standalone v0.4.1: guarded fight entry
runtime cached in RAM; no gameplay I/O
protected module ranges: 0x00000002
Jin move table candidate found; waiting for battle stability
Jin battle stable for 2 seconds; checking master targets
master preflight passed
user_main suspended; beginning master initialization
master initialized (2 signatures)
user_main resumed
```

**Important distinction:** This demonstrates that the v0.4.1 master interpreter completed and resumed `user_main`, but **does not prove the runtime interpreter did not run**. v0.4.1 never logged normal runtime entry/completion, so its absence is inconclusive. v0.4.2 adds one-time runtime boundary logging.

## Additional original EBOOT / master-source inspection

- Decrypted user-supplied EBOOT is ELF32 little-endian MIPS, load offset `0x1018`, load vaddr `0x08804018`.
- Return site `0x0889691C` follows a `jal 0x0896DFF8` at `0x08896914` (`0x0E25B7FE`). Callee `0x0896DFF8` is a simple accessor of guest `0x08C294D4` (`lui v1, 0x08C2; jr ra; lw v0, 0x94D4(v1)`). This helps locate code execution but **does not identify which pointer became `0x10` or why**.
- Original 511,607-line master source parsed from the locally supplied, hashed INI contains **511,199 type-2 32-bit direct writes**, **307 type-E conditional skips**, and **101 type-0 writes**. It has **no type-6 pointer instructions**; prior speculation about type-6 runtime interpreter faults cannot explain a corruption occurring solely within the master pass.
- Master direct targets are primarily populated payload regions `0x090..`, `0x091..`, `0x0A0..`, `0x0A1..`, with 198 repeated writes to game-state `0x08E410D8` and `0x08E410E8`, and 101 zero-type directives mapping to `0x08800000`. No direct master target corresponds to `0x08C294D4` or `0x0889691C`. Indirect effects and timing remain plausible; the precise fault is **not proven**.
- Reviewed PPSSPP `Core/CwCheat.cpp` interpreter: `0xE` conditional comparisons skip N physical cheat records; the source's master opcode composition is simple, but its *application timing* and emulator JIT/cache behavior differ from our PRX. There is still no byte-for-byte Android memory diff proving parity with the working native cheat loader.

## v0.4.2 changes and limits

- Standalone `plugin-standalone/src/main.c` now reads the old auto-cheat sentinel `[0x0A1FFFC0]` before initializing the master. If it is **1 or 2**, it **refuses the new master pass** and records a `CONFLICT` log.
- If both early/late original master data signatures `[0x09110000]=0x08B98190`, `[0x0A1F0AB0]=0x00DF7C0E` already exist **before** the plugin pass, it also refuses duplicate initialization, rather than racing a second writer.
- Logs `first runtime pass beginning` and `first runtime pass completed` **once**, allowing us to distinguish a master-induced vs runtime-induced crash from the next exact log.
- This remains a wholly independent PRX and requires **no PPSSPP Cheats**; the official ultrawide PRX and manifest are untouched.
- This is **defensive diagnostics**, not a demonstrated fix for the `PC=0x10` failure; old native CWCheats could still race if the global Cheats feature remains ON.
- Converted original 511,607 master and 2,458 runtime records are byte-for-byte unchanged, and HD fonts remain low priority.

## Build and installable archive

- PRX source guard commit `a84f367156a8ea5916ad30d59e5ef2214c02a881`.
- PSPSDK build [run 38037496091](https://github.com/zxyandreay/tekken6-jin8/actions/runs/38037496091) — **success**.
- PRX SHA-256 `6b9f54c54df8cf86b581a7e8b943554aa01bdd88bf255b0e16f12fa0296b8c17`; resident ELF memory `0x7A94`, file 37,522 bytes.
- Private user-provided moveset bundle `Tekken6Jin8-v0.4.2-ConflictGuard-NoCheats-ULUS10466.zip` size **2,424,753 bytes**, SHA-256 `c9135a6ae3dce185712ffe665eb45197d85cd6630164440f8b9f4db7bad5c7c9`.
- Resources unchanged: `master.j8c` SHA-256 `956b045210749885ad21da15c0c190f5d6db34fdd8fb998c517c5097fb4c37ca` and `runtime.j8c` SHA-256 `ace847db19328eca86b05412d9ee8f4cc1349ec34ea97f0630c7f570dc7a68aa`.
- User-specific mod data remains excluded from public Git history; PRX source, generator, and incident findings are in the repository.

## Next single clean test (do not loop crashes)

1. Back up game saves and `PSP/Cheats/ULUS10466.ini`. **Set PPSSPP Settings → System → Enable cheats OFF globally.** If installed, remove/disable older `Tekken6Jin8Gate` and restore a pre-v0.3 INI or explicitly remove its two `_C1` auto groups.
2. Close PPSSPP completely; remove prior `PSP/PLUGINS/Tekken6Jin8` and install only the new standalone v0.4.2 ZIP. Leave the official `Tekken6Ultrawide` unmodified.
3. Cold boot. Stop if crash, gather **fresh** `Jin8.log` and crash screen. If it logs a `CONFLICT`, do not work around the guard by repeatedly enabling master.
4. If **Cheats is already definitely OFF** and the exact `PC=0x10` recurs **after master resume**, do **not** simply add longer delays; the next engineering step is a verified master application strategy with native CWCheat semantic parity and controlled guest game-state activation. Avoid claiming the root fixed.

**Status: v0.4.2 source compiled, Android gameplay unverified.**
