# Experiment 005 — cinematic transition found while opponent interfered

**Status: CINEMATIC MEMORY STATE CONFIRMED; VISUAL/CAUSE NOT PROVEN.**

Three user-supplied `jin8-move-condition-watch-v1` files, captured 2026-10-10 with Tekken 6 USA `ULUS10466`, PPSSPP `v1.20.4`. The user did **not** deliberately reproduce slow-motion. Opponent P2 was attacking during nominal `idle`, creating uncontrolled hit/block/move states. The `slowmo` filename describes an **attempted** recreation, not an observed slowdown. Raw files are omitted from Git (machine-state/private diagnostics).

## Input artifact fingerprints

| Original capture name | Samples | Captured span | SHA-256 |
| --- | ---: | ---: | --- |
| `jin8-watch-idle.json` | 20 | 10.986 s | `c8088abe271fe68ae0608374ee76514470c978bd44fe4920a74553bdb37bfc29` |
| `jin8-watch-attack.json` | 40 | 20.643 s | `e896d98d236972d885d733d1d62eef01f2852871b62692a3027beae8cde18d6f` |
| `jin8-watch-slowmo.json` | 40 | 21.759 s | `9010bf0da93c501185dc895df94f49c4436db13b735d21656e231138adbcf827` |

## Runtime pointer link and move observations

- P1 character ID `0x09` and moveset base `0x086A8800` across **all 100 samples**. Every record reports `pointerStableDuringSample=true`.
- The same **three custom pointer values** at source lines `511791`, `512830`, and `513124` match in **20/20 idle**, **40/40 attack**, and **40/40 attempted-slowmo** samples (total 100/100 for each). The other four tracked conditional links remain unmatched in all 100 samples. **Persistence of a value does not imply 100 repeated writes.**
- Nominal idle includes **eight** distinct current move IDs: `0x8001`, `0x01D0`, `0x01A6`, `0x0297`, `0x01D7`, `0x0026`, `0x00A3`, `0x0199`; `incomingMoveId` is nonzero in 2 samples. Because P2 was attacking, this is *not* a stationary untouched idle control.
- Attack includes **14** distinct current move IDs; attempted-slowmo includes **19**. Neither watch samples **current** `0x0266`, `0x043D`, or `0x02E3`. One last attempted-slowmo sample records **incoming `0x0266`** at about **21.759 s**, with **current `0x0000`**; its gated pointer was still zero. This is insufficient to prove the runtime gate did or did not execute.

## Strong new clue: the attack capture contains exact source SlowMo values

At attack samples index **34, 35, 36** (elapsed **17.907, 18.480, 18.992 s**), while `currentMoveId=0x01D7`:

| Index / time | Speed-control guest word | Camera-speed float | Camera-zoom float |
| --- | --- | ---: | ---: |
| 34 / 17.907 s | `0x00000003` | 0.498 | 32.932 |
| **35 / 18.480 s** | **`0x00000003`** | **0.500** | **16.000** |
| 36 / 18.992 s | `0x00000003` | 0.498 | 32.932 |

Source `(1)jin` lines **514426–514437** explicitly label `SlowMo v2.3 Master` and contain **`_L 0x2056F224 0x41800000`** (camera zoom 16.0), **`_L 0x2056F270 0x3F000000`** (camera speed 0.5), **`_L 0x20452890 0x00000003`** (speed mode 3). An additional `SlowMo on K.O.` block at 514030–514065 also writes speed mode 3 and camera speed 0.5, but does **not** contain that zoom=16 line. The three-way exact match strongly supports activation of source-cinematic effects (most specifically the labelled SlowMo v2.3 block), although it does **not** independently prove the conditional gate's precise cause or the visual slowdown.

- Speed mode is `1` for **20/20 nominal-idle**, **37/40 attack**, and **40/40 attempted-slowmo** samples.
- Zoom remains approximately **50** in nominal-idle and attempted-slowmo; attack has the brief drop to **16** and recovery.
- Camera instruction raw guest word `0x3C013F80` remains **identical in all 100 samples**. This matches the corrected `memory.read replacements:false` method and makes the former `0x682CE4C8` read likely an emulator code replacement artifact. Do not confuse live zoom/state variables with the camera instruction word.

## Sampling limitations

Configured interval: **200 ms**, but measured mean start-to-start intervals were **578 ms idle, 529 ms attack, 558 ms attempted-slowmo**, due to remote read overhead; short-lived move IDs/cheat gates can easily be missed. The captures are sequential and the CPU remained active. An observed current move ID `0x01D7` during the event is a **correlation**, not proof that move invokes SlowMo; P2 hit/KO/cinematic circumstances may be relevant.

## Next controlled test

Use **Practice mode with the opponent set to Stand/Do Nothing and no automatic attack**, if that configuration is available. Keep the same ultrawide PRX, `memory=80`, cheats and camera settings. In a stable, already initialized Jin match, try **one repeatable Jin move**, separately recording exact input and outcome.

New **read-only** `watch-jin-cinematics.mjs` monitors fewer relevant game/cinematic control fields and automatically marks an event when `speed mode=3` or zoom drops below 40, recording at least 8 post-detection snapshots. The monitor itself does not alter PPSSPP or make events happen. Run once on an untouched Practice idle control and again while testing one deliberate attack. Do **not** toggle master repeatedly to induce an event. If not reproduced, the negative test is still useful. Source-specific conditional moves require further tracing after the trigger and physical input are controlled.
