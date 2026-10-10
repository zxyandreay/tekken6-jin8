# Experiment 010 — KiwiYu v0.1 appears stock; v0.2 activation telemetry and optional cheat bundle

**2026-10-10.** User reports that the independently compiled KiwiYu Jin 1.0 standalone v0.1 **appears to play like default Tekken 6 Jin**. No `KiwiYu.log` from that device test was provided, so the current evidence **does not establish whether the plugin activated at all**. Do not call v0.1 a verified moveset port.

## Main blocker: successful PRX build is not proof of gameplay writes

The standalone plugin deliberately fails closed until the pointer gate and timer progression pass. The existing v0.1 log can already indicate `waiting for active round timer` vs `both KiwiYu masters initialized`. This is unverified on the user's phone; guessing the state would be inaccurate.

v0.2 is a **diagnostic progression, not a claimed gameplay fix**:
- Round timer's first sample and periodic (~6 seconds) values now logged while waiting, plus `NO MASTER WRITES` if the timer fails to qualify.
- Logs `round timer qualified at` before initializing masters.
- Tracks changed actual guest writes via `mark_dirty` and logs the first runtime pass's changed-write count. A completed first runtime pass with zero changes is now explicitly flagged.
- Maintains original two Kiwi masters (`436,692` + `437,310` commands) and `JIN Moves` (`5,113` commands), the battle timer fail-closed gate, 16ms runtime, and independent PSP plugin module.
- Because no Android KiwiYu log has been received yet, source interpretation, game-state condition, and original source accuracy are *still* not verified as a working Jin conversion.

## Optional cheat groups preserved

The user's source archive `Jin 1.0 by KiwiYu.zip` contains exactly four additional original disabled groups:
- `No MUSIC` — 2 records
- `P1 Rage Turns On` — 8 records
- `Turn off Hazards` — 2 records
- `Heat Meter (Alboss's Textures Required)` — 108 records

The author statement supplied by the user documents Wall Blast, Wall Bound, Floor Blast and Hard Wall Break locations for an **unmodded ISO**, but the archive includes only one general `Turn off Hazards` two-record group; there are **no per-stage hazard switch groups** in the source.

The original camera cheat groups from the **unchanged** [Ultrawide cheat file](https://github.com/zxyandreay/tekken6-ultrawide/blob/main/ULUS10466.ini) are included verbatim:
- `Camera - Original (Default)`: `_L 0x1015350C 0x00003F80`
- `Camera - Slightly Wider`: `_L 0x1015350C 0x00003F81`
- `Camera - Wider`: `_L 0x1015350C 0x00003F82`
- `Camera - Widest`: `_L 0x1015350C 0x00003F83`

The four source optional groups + four camera groups make **eight `_C0` disabled by default**, with **no `_C1`** and **no original Kiwi master/runtime cheats**. Standalone core moveset needs no PPSSPP Cheats. To use these optional groups the user must enable global Cheats, but **do not enable the old core `JIN Moves` _C1** at the same time.

### Compatibility collision identified, not resolved

The user-provided Kiwi `JIN Moves` contains **17 direct runtime writes to guest `0x0895350C`**, overlapping all four camera presets' destination. Kiwi values include `0x3F2D`, `0x3F5A`, `0x3F6A`, `0x3F80`; original camera presets use halfword `0x3F80..0x3F83`. Turning on camera CWCheats while Kiwi runtime writes cinematics may overwrite the Rage Art camera or cause visible conflicts. **Do not claim camera options are conflict-free.** Test Rage Art with optional camera cheats OFF first; leave official Ultrawide PRX enabled for aspect/HUD correction.

## Built private artifacts and verification

- CI [standalone PRX v0.2 build 38049881978](https://github.com/zxyandreay/tekken6-jin8/actions/runs/38049881978) **success**; research checks successful on commit `c122eac5a2a25e86843283162524abb45cb81158`.
- Compiled `Tekken6JinKiwiYu.prx`: 42,250 bytes, SHA-256 `c21ceb90420c6cf774cdca966d7aebc1610eca9c04b54ff5520e223a5ffa3126`.
- User-specific full archive: `Tekken6JinKiwiYu-v0.2-Core-and-OptionalCheats-ULUS10466.zip`, **4,062,421 bytes**, SHA-256 `5d7e3dd7a0d190df5be0b56f0114145a9a7e54b0d39fcb54393a0aba06ddd388`.
- User-specific optional-only ZIP: `Tekken6JinKiwiYu-v0.2-OptionalCheats-UltrawideCamera.zip`, **4,430 bytes**, SHA-256 `6bd01ccc02ade58a17f830de647bf92b73631401f57e3ea24d672d73519d1fff`.
- Combined optionals-only cheat INI: 3,815 bytes; SHA-256 `1a03ae6523818975ece7d4e73605fc6e3fbeb7c43c25e00ec0916ed3fec0522e`.
- Original `master` / `runtime` resource bytes remain unchanged from v0.1.
- Full ZIP contains `PSP/Cheats/ULUS10466.ini`; installing directly **overwrites any existing game cheat INI**. It also contains `OPTIONAL-CHEATS/ULUS10466.ini` so users can manually merge and preserve custom codes. All optional cheats are disabled until enabled by the user.
- No original Kiwi Yu master/moveset data publicly added to GitHub. User supplied author's terms: **no commercial use**, credit Kiwi Yu, direct source redistribution to original server (URL not supplied); do not invent author's server link.
- Official `tekken6-ultrawide` public repository, release and binary remain unchanged.

## Acceptance / next evidence required

1. Close PPSSPP, back up saves and `PSP/Cheats/ULUS10466.ini`, disable previous Jin8 and KiwiYu v0.1 plugin, install the full v0.2 package. Keep official ultrawide enabled and **global Cheats OFF for initial core test**, optional groups therefore dormant.
2. Select P1 Jin in a normal timed round. If stock Jin remains, provide **`PSP/PLUGINS/Tekken6JinKiwiYu/KiwiYu.log`** so we can determine whether the timer gate blocked every write, master preflight failed, or runtime executed with zero actual changed writes. Do not claim success merely because match loads or the PRX compiles.
3. Only after custom moves are confirmed, test `d/f+1+2` at low HP/Rage; user reports this as the original Kiwi input but code author's archive does not explicitly document it. With optional camera cheats OFF, verify cinematic.
4. Optional cheats can be enabled individually if desired by setting global Cheats ON. At most one camera preset. Heat Meter needs separate Alboss textures. The general hazards-off cheat disables stage hazards together, not independently.
5. If v0.2 cannot progress past a stable stock match, next work should be a targeted activation-gate or pointer-condition correction based on log, **not another blind rewrite**.

**Status: experimental; no real-device proof that converted moveset is active.**
