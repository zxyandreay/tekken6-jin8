# Incident 012 — KiwiYu v0.3 partial gameplay, stutters, freezes and cinematic camera (2026-10-10)

**Status:** user-supplied v0.3 log shows master A+B and runtime executed. Source-backed mitigation v0.4 compiled and packaged; **no Android v0.4 validation yet.** Do not claim stutters are fully fixed.

## New user feedback and exact log

User: "now somehow it looks like it's kinda working but there's stutters and freezes in certain moves. there's also camera zoom-ins and gameplay slow-motions. it still seems unstable."

Complete `KiwiYu (1).log` (v0.3):

```text
KiwiYu Jin 1.0 standalone v0.3: verified-fight input fallback
runtime cached in RAM; no gameplay I/O
protected module ranges: 0x00000002
Jin move table candidate found; waiting for battle stability
stable move table; waiting for active round timer (no writes yet)
round timer first sample = 0x00000000
P1 move corroborated = 0x000001EE
P1 gameplay input corroborated = 0x00FD0200
PLAYER ACTION CONFIRMED: actual P1 input+move; master may initialize
in-fight activity or countdown confirmed; checking master targets
master preflight passed
master preflight passed
user_main suspended; beginning KiwiYu master A
master JIT invalidation completed
KiwiYu master A completed; applying master B
master JIT invalidation completed
both KiwiYu masters initialized (2 signatures)
user_main resumed
first runtime pass beginning
first runtime pass completed
first runtime changed write operations = 0x000002C7
```

`0x2C7=711` **changed** writes in the first runtime pass. This proves the new core actually wrote data and makes the earlier v0.1–v0.2 stuck timer-gate diagnosis obsolete. It does not prove all animation paths, Rage Art, or game stability.

## Four concrete code/source findings

1. **Huge conditional copies in Kiwi Yu's original source.** The `JIN Moves` group contains **16** occurrences of `_L 0x50300000 0x00880000`, immediately followed by `_L 0x00400000 0x00000000`. Under PPSSPP's base-offset semantics they copy **0x880000 = 8,912,896 bytes (8.5 MiB)** from guest `0x08B00000` to guest `0x08C00000` when preceding `E1` conditions pass. Each copy can invalidate a giant region and overwrite live game data. **Conditional execution, not confirmed on the phone**. The same source blocks also use `_L 0x20407D24 0x12345678` (pointer placeholder). A single executed block could cause severe pauses or corrupt game pointers.
2. **v0.3 JIT invalidation overflow.** The PRX tracked up to **128 disjoint dirty address spans**. Once they overflowed it fell back to **one huge min-to-max `sceKernelIcacheInvalidateRange()` call**. With 711 changed write operations during the first runtime pass, disjoint span overflow was **plausible but not proven**. This invalidates JIT code in a very wide range, leading to needless recompilation/hitches.
3. **Premature reinstallation risk.** The old state loop reset `installed=0` after **40 × 25ms (~1 second)** of missing Jin move pointers; new normal pointers then retriggered **both 874,002-record masters** while still in fight. Move pointers can be transient during cinematic transitions. An observed per-move repeat is **not proven**, but this was an objectively unsafe design for smooth gameplay.
4. **The creator *intentionally* scripts camera/interpolation changes.** Original `JIN Moves` has **17 direct writes** to camera instruction `0x0895350C`, with values `0x3F2D, 0x3F5A, 0x3F6A, 0x3F4B, 0x3F80` etc. The `0x08D6F270` controller takes values `0x3DCCCCCD, 0x3F000000, 0x3E800000` during special sequences. These source-authored effects are **not exclusively a bug**, but may be undesirable during normal gameplay and compete with ultrawide's optional camera presets. Do not assert `0x08D6F270` is the whole game-speed engine without proof; it's documented as camera/interpolation in existing snapshots.

## v0.4 SmoothGuard implementation

**Keep the two exact Kiwi Yu masters + Jin Moves binary resource bytes UNCHANGED. No source commands removed or repacked.** Official ultrawide unchanged.

- New `plugin-kiwiyu/src/dirty_pages.h`: 2,560-byte bitmap covering the 80MiB guest window as **20,480 pages × 4KiB**. After each runtime pass, invalidate only touched pages, merging adjacent ones. The giant overflow fallback is **removed**. For the one-time 874,002-record master initialization, retain one batched range invalidate per master.
- During runtime, **block 8.5MiB source snapshots by default** (size greater than 1MiB). Also block their companion `0x12345678` write at `0x08C07D24`, so the pointer is not corrupted when the copy is suppressed. Advanced explicit marker `KiwiYu.allowlargecopies` restores original behavior and is not recommended for standard gameplay.
- For all smaller direct and pointer-derived copies, compare source/destination and only `memmove` + invalidate when actual bytes differ.
- **Smooth camera mode default:** skip KiwiYu's runtime writes to `0x0895350C` and `0x08D6F270` while preserving all other original instructions. Explicit empty marker `KiwiYu.cinematics` restores original cinematic zoom/interpolation. This may change original special camera choreography or Rage Art; new stable baseline comes first. It does **not** guarantee all slow motion is gone because other effect triggers may remain.
- Require **320 × 25ms (~8 seconds)** of missing active fight pointers to re-arm master installation, rather than 1 second; no longer reinstall simply because the moveset base pointer changes transiently. One `total master initializations` counter per running session.
- New runtime log diagnostics: invalidated JIT byte count, number of batches, changed-write count, big-copy and sentinel/cinematic skips on first runtime; summarized every 256 runtime passes rather than per frame.

## Verification

- PSPDEV compile [GitHub Actions 38052785115](https://github.com/zxyandreay/tekken6-jin8/actions/runs/38052785115): **success** including independent host dirty page boundary test and P1 activity gate test.
- General research tests [Actions 38052785094](https://github.com/zxyandreay/tekken6-jin8/actions/runs/38052785094): **success**.
- v0.4 PRX SHA256 `6f5d830681d3014b5ccaabba24e2d8f12f1fea7f559691afa3c17ac806220252`, 46,026 bytes, mapped footprint `0x9C14` (39,956 bytes).
- User-specific install ZIP `Tekken6JinKiwiYu-v0.4-SmoothGuard-OptionalCheats-ULUS10466.zip`, 4,064,824 bytes SHA256 `858253914b567e2757537b61900fb68dbb892dc7d7e8be7a43f3aa74b3ea3551`, 11 files. ZIP integrity and per-entry binary equivalence checked.
- Both original `jin_master.j8c` and `tekken8_master.j8c`, original `jin_moves.j8c`, and `PSP/Cheats/ULUS10466.ini` are **byte-for-byte identical to user-specific v0.3 package**. All 8 optional cheat groups remain `_C0` OFF, and the independent official Ultrawide PRX is not modified or copied over.
- **No Android v0.4 result yet**; static/CI tests alone cannot certify Rage Art, frame pacing or texture compatibility.

## User test

Back up saves and especially `PSP/Cheats/ULUS10466.ini` before extraction (full ZIP includes a replacement optional file). Disable previous Jin8/KiwiYu versions, leave unchanged official `Tekken6Ultrawide` v1.2.1 enabled, turn global Cheats **OFF** for the initial test. Cold boot normal timed Jin P1 fight, trigger once using ordinary attack after stage ready. Compare movement smoothness and previously freezing special moves; test Rage Art `d/f+1+2` in Rage. Keep `KiwiYu.cinematics` and `KiwiYu.allowlargecopies` absent initially. If any crash, stop and supply screenshot + `KiwiYu.log`; otherwise supply new log and note missing moves/effects. Only retry creator's cinematic marker if baseline is stable; NEVER recommend opting into large copies casually.

Source credit: Kiwi Yu, original code user-provided; creator asked for noncommercial use and for public sharers to link the creator server, address unknown.
