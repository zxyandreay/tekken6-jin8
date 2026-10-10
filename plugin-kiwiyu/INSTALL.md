# KiwiYu Jin 1.0 v0.4 — SmoothGuard

**Experimental standalone PRX, not yet Android validated.** Builds from the user's original Kiwi Yu source, with both master streams and full JIN Moves unchanged. No PPSSPP native Cheats required for core. The official Tekken6Ultrawide remains unchanged.

## Why this version exists

The user-confirmed v0.3 `KiwiYu (1).log` shows both masters initialized, first runtime executed, and 711 changed writes, but certain moves had stutters/freezes and scripted zoom/slow-motion. An audit found 16 conditional 8.5MiB guest-memory copy commands, a linked invalid pointer sentinel, an enormous JIT invalidation fallback when 128 dirty ranges overflow, and the possibility of master reinitialization after just 1 second of temporary pointer disappearance.

## New default safety policy

- Exact 4KiB-page JIT invalidations during the ~16ms runtime; deduplicated bitmap prevents a blanket invalidate when 128 disjoint addresses are exceeded.
- Block each runtime memcpy greater than 1MiB (16 possible 8.5MiB instructions in original Kiwi Yu source) and suppress linked `0x12345678` pointer write.
- Skip small and pointer-derived memcopies when source and destination are already identical.
- Wait ~8 sec of missing Jin fight state before re-arming the 7MB master loader; do not reinitialize on incidental pointer/base changes.
- Default **smooth camera**: suppress two *known* source camera/interpolation destinations `0x0895350C` and `0x08D6F270`. All other animation and special-move instructions are unchanged. This can modify original Rage Art choreography. Other speed effects may still occur.
- All optional original Kiwi Yu cheats and ultrawide camera presets remain opt-in as separate `_C0` entries.

## Install and test

1. Back up your PPSSPP saves and particularly `PSP/Cheats/ULUS10466.ini`. The complete user-specific ZIP includes an optional `PSP/Cheats/ULUS10466.ini` and may **overwrite your existing game cheat list**. Merge manually if you have other entries.
2. Fully close PPSSPP. Remove any previous `PSP/PLUGINS/Tekken6JinKiwiYu` folder (including old markers), disable older Jin8/Gate variants, and extract the v0.4 folder. Only enable KiwiYu v0.4 and the official **unchanged** Tekken6Ultrawide plugin.
3. Keep global Cheats **OFF** for the first comparison. Select Jin P1, timed battle, wait until controllable, then press an ordinary attack once to satisfy the established gameplay readiness gate. One initial master setup can pause briefly, but recurrent freezing is not expected.
4. Test ordinary combos and moves that previously froze; then Rage Art using `d/f+1+2` in Rage. Look at `PSP/PLUGINS/Tekken6JinKiwiYu/KiwiYu.log` for new JIT/largecopy/camera diagnostic counts.
5. If crash, stop immediately and send screenshot and log. If stable but any moves/cinematics missing, send log and which move was affected.

## Optional compatibility markers (disabled by default)

In `PSP/PLUGINS/Tekken6JinKiwiYu/` create an empty file with these exact names **only if you intentionally want to test the matching feature**, then cold boot:

- `KiwiYu.cinematics`: opt back into Kiwi Yu's authored camera zoom and interpolation effects. Test only *after* verifying smooth baseline.
- `KiwiYu.allowlargecopies`: opt back into 16 source conditional 8.5MiB memory snapshots + invalid pointer sentinel; **HIGH RISK** of freezes/crashes. Do not use in normal play.

Neither marker is bundled/enabled by default. The full ZIP retains the same separate optional `_C0` cheats (No MUSIC, P1 Rage Turns On, Turn off Hazards, Heat Meter requiring Alboss textures, four official Ultrawide camera presets). If enabling optional native cheats in future, enable at most one camera preset and never load original Kiwi master/moveset `_C1` code alongside PRX.

## Status and credits

Automated PSPDEV compilation and host regression checks passed. Android stability and Rage Art functionality **not yet verified**. Kiwi Yu retains original code authorship; respect the creator's noncommercial/source-linking request. See [incident 012](../research/jin-lifecycle/incident-012-v04-smoothguard.md).
