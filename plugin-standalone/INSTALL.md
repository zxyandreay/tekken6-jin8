# Tekken6Jin8 v0.5 — Battle-clock-gated standalone PRX

**Experimental build, not yet confirmed stable on Android.** This is a fully automatic PRX with **no PPSSPP Cheats required**. The original ultrawide plugin does not need modifications.

Previous master-only tests with Cheats OFF crashed at PC=0x10 **after master initialization**; therefore the old arbitrary 2-second stable-moveset check was inadequate. The v0.5 loader will not touch the master until **actual battle-timer progression** is observed. A static timer means **no writes** (default Jin / fail closed).

## Clean installation

1. Fully close PPSSPP and back up saves and relevant game settings.
2. Remove `PSP/PLUGINS/Tekken6Jin8/` and `PSP/PLUGINS/Tekken6Jin8Gate/` from **earlier experimental versions**, including old `Jin8.masteronly` and `Jin8.safe` markers.
3. Extract the user-specific ZIP into your **PPSSPP Memory Stick root**. Expected Jin8 folder contains `plugin.ini`, `Tekken6Jin8.prx`, `master.j8c` and `runtime.j8c`.
4. PPSSPP Settings → System → **Enable Cheats OFF** globally. You do **not** need to alter `PSP/Cheats/ULUS10466.ini` for this plugin.
5. In Plugin Manager enable **Tekken 6 Jin8 Standalone v0.5** and your existing official **Tekken6Ultrawide**. Do not touch ultrawide PRX/manifest.
6. Cold boot Tekken 6 USA; select Jin as Player 1 and enter a timed fight. The plugin may wait until the **round has actually started** before installing the data.
7. If custom moves work, assess smoothness after the first installation. Look at `PSP/PLUGINS/Tekken6Jin8/Jin8.log` for exact status.

## Important log messages

- `stable move table; waiting for active round timer (no writes yet)` — no master writes have occurred; may mean the battle hasn't started or the timer signal is unsupported in this mode.
- `ROUND TIMER ACTIVE: battle confirmed; master may initialize` — the gate observed repeated plausible timer decreases.
- `master initialized (2 signatures)` — the two sampled data signatures matched after attempted master execution; this does **not** certify the entire moveset.
- `first runtime pass completed` — runtime has started applying the existing conversion instructions.

If the fight remains stable but normal Jin's moves are unchanged, do not keep toggling unrelated cheats. Send the new log; an unsupported game timer can make the plugin **intentionally do nothing**.

If a crash recurs, stop and send the crash screenshot plus new log. Do **not** repeatedly restart into the known failure.

`Jin8.masteronly` is available as an optional internal diagnostic, but keep that marker **absent** for normal testing. `Jin8.safe` disables mod writes.

[Detailed v0.5 research and original master-only crash log](../research/jin-lifecycle/incident-008-v05-battle-clock.md).
