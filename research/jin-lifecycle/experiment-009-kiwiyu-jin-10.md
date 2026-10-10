# Experiment 009 — KiwiYu Jin 1.0 replacement-source prototype (2026-10-10)

**Status:** v0.1 experimental PRX compiled successfully and a complete user-specific ZIP was assembled. **Not yet run on Android; no claims of crash-free gameplay or working Rage Art.** This is a NEW, separate plugin directory; the existing Jin8 v0.5 branch and official ultrawide project are left untouched.

## Uploaded source and creator guide

- User-supplied ZIP `Jin 1.0 by KiwiYu.zip` — SHA-256 `345941b9a24736a15af9a141f709ebbdd88874faf379717c3b7a153b287a1220`, approx 4.5MB, contains `ULUS10466.ini` (21,981,034 bytes) and `Jin By KiwiYu Guide.md`.
- INI SHA-256 `5a1e6eb63e50341d35bc5d8d283d499d0491420eb3bd7f66e183b596320b6193`.
- Creator's guide states: "**Make sure your PPSSPP refresh interval is set to 16**" and "**Plugins required for the best experience [Must have]**". It does **not name the required plugins**, nor identify the Rage Art input.
- The user states that Rage Art is **confirmed via `d/f+1+2`** in this particular source mod. That is user-provided functionality information, **not independently confirmed by the ZIP's guide or an Android PRX test**.
- Source is clearly different from prior `ULUS10466 (3).ini`: it has two `_C0` master groups and one `_C1` continuous runtime group, rather than the prior master/runtime pair.

## Exact source groups

| Original group | Original activation | Records | Included by default | J8C1 resource |
| --- | --- | ---: | --- | --- |
| JIN MASTER CODES | `_C0` | 436,692 | yes, one-shot master A | `jin_master.j8c` |
| TEKKEN 8 MASTER CODES | `_C0` | 437,310 | yes, one-shot master B | `tekken8_master.j8c` |
| JIN Moves | `_C1` | 5,113 | yes, cached ~16ms | `jin_moves.j8c` |
| No MUSIC | `_C0` | 2 | no | excluded |
| P1 Rage Turns On | `_C0` | 8 | no; optional forced-Rage code | excluded |
| Turn off Hazards | `_C0` | 2 | no | excluded |
| Heat Meter (Alboss's Textures Required) | `_C0` | 108 | no; requires textures | excluded |

- One source physical line, **799754**, contains two `_L` instruction pairs accidentally concatenated. The converter recovers both **in original sequence**. Total `TEKKEN 8 MASTER CODES` is 437,310 *logical* instructions, not 437,309 text lines.
- Three retained streams total **879,115 records** (874,002 master + 5,113 runtime); serialized bytes **7,032,944** plus headers already included in per-file lengths.
- Type mix: first master exclusively 436,692 type-2 direct writes. Second master: 437,256 type 2, eighteen each type E, type 6 and type 0. Runtime: 2,139 type E, 1,148 type 2, 922 type 0, 692 type 6, 122 type 5, 50 type 3, 36 type 1, **four type D**.
- Existing v0.5 standalone interpreter **did not implement type D**, meaning a direct resource swap would fail. v0.1 implements D4-D7 address comparisons, consuming the following physical record and conditionally skipping the specified count, based on PPSSPP `Core/CwCheat.cpp`.
- Two master sentinel values: `[0x0B000000] == 0x00008001` and `[0x0A200000] == 0x08B98190`. These are weak sampled confirmations, not full semantic parity.
- All direct writes in the two master groups are within the PRX expanded memory guard bound `< 0x0D000000`, subject to runtime module overlap checks. The one-shot master writes are a much larger pause/crash risk than the previous payload.
- The established round-timer gate at `0x090FBE4C` is not directly overwritten by this source's master writes. Actual round-timer address accuracy for KiwiYu's conditions still needs an Android test.

## Files/PRX

- Separate `plugin-kiwiyu` source path and PSP module `Tekken6JinKiwiYu`, separate `KiwiYu.log` and installed folder. Previous `Tekken6Jin8` must be DISABLED, never active simultaneously.
- Automatic master A followed by B once per fight under suspended game `user_main`; two weak signature checks, then ~16ms cached `JIN Moves` runtime. The original game-source instructions remain unchanged; only interpreter includes type-D support.
- Explicit `memory = 80` in manifest; PPSSPP Cheats must be globally **OFF**; original ultrawide plugin can remain enabled **without modifications**.
- Rage Art button `d/f+1+2` is a **user-reported input to test**, not claimed proven through PRX.
- Source code is public; user-owned original master/runtime binaries were not added to GitHub. Generic source converter `tools/build_kiwiyu_resource.py` reproduces their binary form from the exact archived source ZIP.

## Validation and artifacts

- Source PRX build commit `c47adf17a075dbaeef74d89b6ce40ffed2d4b479` (after fixing module self-identity and adding PSP exports).
- Successful [GitHub Actions run 38048695435](https://github.com/zxyandreay/tekken6-jin8/actions/runs/38048695435).
- PRX `Tekken6JinKiwiYu.prx` 41,122 bytes, mapped 35,060 bytes, SHA-256 `40670b8f2f779c73ef15ffac8f296a4d4c58c0ccd198dc0d1ab0514f030548c5`.
- `jin_master.j8c` 3,493,544 bytes SHA-256 `be00e6a1d17d5e737cde9613f2b4d4d1e6c224e431aed0b63f3333fbbf318a91`.
- `tekken8_master.j8c` 3,498,488 bytes SHA-256 `2b7be69781695fb7fb7dbcdf9115591088b7e9ea8ad5a87bc2bc4b3c2c767f0c`.
- `jin_moves.j8c` 40,912 bytes SHA-256 `101b882a8509a10ab0fcbb8266bfb1d69b7cf1759b91f3812bf7aae172a07197`.
- Verified user-specific ZIP `Tekken6JinKiwiYu-v0.1-Standalone-NoCheats-ULUS10466.zip`: 4,056,737 bytes SHA-256 `a7b8240c07cf397810874d7b36fd5b2c106b9571e938761ee94722c4076b756e`.
- ZIP CRC and each entry SHA verified locally. No Android user run yet; **a compiled PRX is not evidence of gameplay compatibility**.

## Acceptance test

1. Back up saves and Memory Stick. Fully close PPSSPP.
2. **Disable older `Tekken6Jin8` v0.5 and `Tekken6Jin8Gate` plugins**. Keep the official ultrawide release unchanged.
3. Extract the new ZIP at PPSSPP Memory Stick root. Enable **Tekken 6 Jin KiwiYu Standalone v0.1** and official ultrawide only, with global Cheats OFF.
4. Cold boot Tekken 6 USA ULUS10466, P1 Jin, normal timed fight. Check if stage loads. Log should show `ROUND TIMER ACTIVE`, `both KiwiYu masters initialized (2 signatures)` and `first runtime pass completed`. If only `waiting for active round timer` appears, it is **not a successful moveset load**, just a safe fallback to stock Jin.
5. With actual conversion confirmed, lower HP to Rage and press `d/f+1+2` (down-forward + Square + Triangle). Assess whether the complete cinematic plays and whether ongoing gameplay stutters. User's Rage Art input is a starting hypothesis for the PRX port.
6. If crash occurs, **stop**, disable experimental KiwiYu PRX and share `PSP/PLUGINS/Tekken6JinKiwiYu/KiwiYu.log` and crash screenshot. Do not return to repeating old v0.4 crashes.

**Caveats:** First master injection is approximately 7MB of instructions, possible fight-start stall. The creator's guide may require other unnamed plugins beyond the user's existing ultrawide plugin. No claim of fully working Rage Art, 16ms smoothness, HD font compatibility or crash safety until real-device test.
