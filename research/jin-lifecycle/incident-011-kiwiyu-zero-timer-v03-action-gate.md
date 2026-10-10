# Incident 011 — KiwiYu v0.2 never initialized; v0.3 gameplay-action fallback

**2026-10-10; state: experimental, user Android gameplay result pending for v0.3.**

## Supplied v0.2 evidence (exact 9 lines)

```text
KiwiYu Jin 1.0 standalone v0.2: activation/write telemetry
runtime cached in RAM; no gameplay I/O
protected module ranges: 0x00000002
Jin move table candidate found; waiting for battle stability
stable move table; waiting for active round timer (no writes yet)
round timer first sample = 0x00000000
round timer after ~6 seconds = 0x00000000
NO MASTER WRITES: waiting for supported countdown
round timer first sample = 0x00000000
```

The user's observation of stock Tekken 6 Jin is **fully explained by this log**. The supposed Tekken 6 round timer `0x090FBE4C`, drawn from a public freeze-timer cheat, reads zero throughout a real match on this user's ULUS10466 PPSSPP. The v0.2 implementation required a strictly decreasing counter at this address and therefore **never wrote either Kiwi Yu master, never executed the 5,113-record Jin Moves runtime**, and never loaded custom moves. The installer compiled successfully but was functionally inactive in this game session. This supersedes speculation that correct data failed to hook into a loaded master.

## Source-backed in-fight alternative (no cheats)

Previously recorded *read-only stock Tekken 6* gameplay states in `jin8-watch-idle.json` and `jin8-watch-attack.json` (user-provided captures of a working P1 Jin match) established:
- `0x08C078B4 == 9`: Player 1 Jin character code
- `0x08C078E0`: valid moveset base and `0x08C07964`: current move pointer (these can appear too early, **not sufficient alone**)
- `0x08C07A18`: current move ID: idle `0x8001`, active attacks e.g. `0x01A6` / `0x01D0`
- `0x08C07D3C`: input state: neutral `0x00FF0000`, active button values `0x00FE0100`, `0x00FD0200` (and similar)

**v0.3 fix:** after ~2 s stable base and current-move pointer, allow original countdown gate *or*, when timer stays zero, require a transition from neutral to a plausible non-neutral input state **plus an actual active move-ID transition**, followed by ~300ms of stable observation. Ordinary controls are used to confirm the fight is playable, not as a manual cheat-trigger or a cheat toggle. An unchanging move pointer, background CPU move movement or nonzero but static timer alone are intentionally insufficient; they **fail closed**.

For the first test ask user to wait until Jin is visually controllable, then hit Square once after ~2 seconds. If missed or too early, one more ordinary move is okay. No PPSSPP Cheats are required for automatic core moveset execution. This design corrects the *known gate failure*; actual safe late initialization of the original two large masters remains unproven.

## Source/data/config preservation

- Original source: user's `Jin 1.0 by KiwiYu.zip`.
- Original converted `jin_master.j8c` (436,692 records), `tekken8_master.j8c` (437,310 records) and `jin_moves.j8c` (5,113 records) are **byte-for-byte unchanged from v0.1/v0.2**.
- Existing v0.2 optional cheat INI retained unmodified: eight disabled-by-default `_C0` groups (four original Kiwi Yu optional extras, four exact ultrawide camera presets). No native core master/runtime cheats duplicated.
- Standalone core still runs with Cheats OFF; optional cheats require PPSSPP Cheats ON only when specifically desired. Camera presets can collide at `0x0895350C` with Kiwi's cinematic camera effects and should be OFF while verifying Rage Art. Ultrawide automatic 3D/HUD PRX and its source remain untouched.
- New code `plugin-kiwiyu/src/battle_activity.h` has a deterministic host test `tests/battle_activity_test.c` reflecting in-fight values; the test verifies no activation from static early-loaded pointers or unauthenticated CPU move transitions.
- GitHub PRX build [38051727290](https://github.com/zxyandreay/tekken6-jin8/actions/runs/38051727290) **success**, including separate host activity-gate test. Research suite [38051794851](https://github.com/zxyandreay/tekken6-jin8/actions/runs/38051794851) **success**, following update of a v0.2 timer-specific log assertion.
- Code commit `9514e878258f496483ccced337790045b4aacff4`; final research-test alignment `ddfb261334c9e6aebe947b84ef110aa57331fd54`.
- User-specific verified archive `Tekken6JinKiwiYu-v0.3-GameplayActivity-and-OptionalCheats-ULUS10466.zip`: **4,062,737 bytes**, SHA-256 `53edf45ce41e014ea8dc74c556a98048db30cbd421c682ba3b7180e29e0c5d8f`.
- PRX `Tekken6JinKiwiYu.prx`: 42,754 bytes, SHA-256 `ca23dbd3d670cf908d964c497ffcfc5be7f10c6f56ec79f15beb18d6b7c1755e`.
- Resources SHA-256: `jin_master.j8c` `be00e6a1d17d5e737cde9613f2b4d4d1e6c224e431aed0b63f3333fbbf318a91`; `tekken8_master.j8c` `2b7be69781695fb7fb7dbcdf9115591088b7e9ea8ad5a87bc2bc4b3c2c767f0c`; `jin_moves.j8c` `101b882a8509a10ab0fcbb8266bfb1d69b7cf1759b91f3812bf7aae172a07197`.
- Zip integrity and every entry were verified locally. Public repo includes only interpreter and converter/diagnostic tests; private source master bytes were not republished.
- Existing camera cheat INI SHA `1a03ae6523818975ece7d4e73605fc6e3fbeb7c43c25e00ec0916ed3fec0522e` unchanged. **The complete ZIP contains `PSP/Cheats/ULUS10466.ini`; extraction can overwrite an existing custom cheats file. Back it up and merge!**

## Critical acceptance limits

This is a **targeted activation fix**, not evidence that two Kiwi Yu masters correctly initialize, that the Rage Art `d/f+1+2` works, that built-in cinematic camera is compatible with all extra camera cheats, or that gameplay is stutter-free. Previous Jin8 v0.4 master-only crashed after initializing under too-early detection, so initial Kiwi Yu master application must be monitored closely; the user should **stop after a single crash**, not keep re-testing a destructive stage.

Expected v0.3 log after a visible P1 action: `PLAYER ACTION CONFIRMED`, `master preflight passed`, `both KiwiYu masters initialized (2 signatures)`, `user_main resumed`, `first runtime pass completed`, `first runtime changed write operations = ...`. If it still says `NO MASTER WRITES`, capture move/input diagnostic values to revise activity detection. If log reaches master but crashes, focus on master semantics/timing rather than creating more gates. Only once moves visibly change should the user try Rage Art `d/f+1+2`.

**Distribution:** respect Kiwi Yu's non-commercial, author-credit, original-server-link conditions; no original server URL supplied or guessed. Official ultrawide release unchanged.
