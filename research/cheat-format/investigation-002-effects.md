# Investigation 002 — unexpected camera, slow motion, and hybrid Jin moves

**Status: SOURCE ANALYSIS CONFIRMED; LIVE FAULT CAUSE NOT DETERMINED.**

A user running the repacked Jin cheat described a mixture of Tekken 8-inspired and stock Tekken 6 moves, unusual camera zooms, and apparent game slow motion. No mod-on JSON captures or PPSSPP perf trace have been supplied for this symptom. **Do not label the overall loader functional or broken from appearance alone.**

## Checked local source and diagnostic ZIP

- Source: `ULUS10466 (3).ini`, SHA-256 `96541d37ad9ec8114c5f07feca63143d90cc0cdcda2218494cb19c970a8603d3`.
- Repack: `Tekken6-Jin8-Experiment002-CWCheat.zip` SHA-256 `07e4be1c58d139648c73fd3cb0d0259db96d881ef27873cceb9fc598db87acf9`.
- Repack contains a single INI, README, manifest; **no PRX, altered ISO or required external mod datapack**.
- Diff check: added `_S ULUS-10466` / `_G Tekken 6 [USA]`; flipped `_C1 (1)jin` to `_C0 (1)jin`. All 514,065 `_L` instruction lines preserved byte-for-byte. The master remained `_C0`.
- Therefore repacking did **not** add the zoom/slow-motion logic, but its author's complete original bundle and exact dependencies were **never verified**. The repack should not be described as a confirmed full Tekken 8 conversion.

## Positive source evidence: intentional effects in the Jin runtime group

Lines in the source (one-based, including commented headings):

| Line | Evidence |
| --- | --- |
| 514030 | `//SlowMo on K.O.` with writes to `0x20407CA0`, `0x20452890`, `0x2056F270` |
| 514053 | `_L 0x1015350C 0x00003F74` (camera halfword) |
| 514288 | `///////SLOWMO/////////` with move-conditional triggers |
| 514399 | `_L 0x1015350C 0x00003F70` |
| 514423–425 | `//DEFAULT CAMERA ZOOM IN` and `_L 0x1015350C 0x00003F80` |
| 514426–445 | `// SlowMo v2.3 Master` and speed writes `0x20452890` / `0x2056F270` |

Public Tekken 6 code-making notes independently document the meaning of `SlowMo v2.3 Master`: conditional game speed and camera speed/zoom changes, including a strong zoom and an approximately one-fifth-speed effect in a documented example. See [Tk6 Cheat Making UPDATE 2](https://pdfcoffee.com/tk6-cheat-making-update-2-pdf-free.html) (public hosted transcription; attribution/unverified revisions). **This does not establish which exact condition triggered in the user's run.**

## Known conflict with ultrawide

- Jin runtime has three separate writes to CWCheat address `0x1015350C` — `0x3F74`, `0x3F70`, `0x3F80`.
- Ultrawide optional camera presets also use `0x1015350C`: `0x3F80` through `0x3F83`.
- This can cause oscillating camera values depending on active cheats, refresh cadence, and game state. The ultrawide **PRX's aspect/HUD routines are not yet proven to collide with Jin moveset memory**. Distinguish optional camera cheat collisions from the PRX itself.

## Why stock/Tekken 8 hybrid moves are ambiguous

- Source includes numerous move-specific sections, but not a proof that all original Tekken 6 moves were replaced.
- Some conversions deliberately retain stock moves. Partial/mismatched activation or missing original prerequisites are also possible.
- Some related community mods require a matched modded game image/datapack and additional master system codes; **that evidence refers to those specific mods, not proof that this Jin pack has identical prerequisites**.
- Never infer all 511,607 master records were processed successfully from a handful of working custom moves. Require a completed and stable A/B capture and actual move test.

## Corrected diagnostic advice

1. Stop replaying the giant master code if zoom/slowmo/freezes make the match unstable. Disable both cheats, fully exit PPSSPP, then relaunch (turning them off does not restore all RAM writes).
2. Restore the original backup before uncontrolled changes. Verify normal stock play with ultrawide and Jin cheats both OFF.
3. For move-mod isolation, **prefer Jin cheats without ultrawide effects**. However, plugin `memory = 80` from the ultrawide manifest only applies if that plugin is enabled. If disabling it makes the custom payload region invalid, **do not enable the giant master**. Prepare a separately tested memory-only provider or another supported expanded-memory configuration first.
4. If retaining ultrawide just to get the memory setting, run a recorded combined-mode baseline, **disable all optional ultrawide camera CWCheats**, and understand that this is **not** a Jin-only control.
5. Keep PPSSPP refresh rate and settings documented; community guides report `16` for *certain related mods*, not confirmed as a required setting for this exact file.
6. Capture reproducible effects in separate states. The passive snapshot now includes a few candidate zoom/speed controller words. No direct modification to source cheat or ultrawide repository is authorized by this finding.

## Follow-up evidence needed

- Original download/source and full setup instructions for **this exact cheat payload**.
- PPSSPP's active plugin list, expansion configuration, cheat refresh rate, and whether optional camera preset is enabled.
- `stock-jin-battle-deep.json` plus `mod-jin-battle-deep.json` (if stable), and ideally a `mod-jin-slowmo.json` snapshot during unusual zoom, with `--inspect-pointers`.
- Specific move/input demonstrating successful Tekken 8 behavior versus a clearly stock move; whether slowdown is FPS/performance or intentional animation tempo.

