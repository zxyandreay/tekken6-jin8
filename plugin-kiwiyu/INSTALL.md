# KiwiYu Jin 1.0 — no-cheats experimental PRX v0.1

This is a new independent standalone plugin; disable previous Jin8 and Jin8Gate. Leave official ultrawide unchanged. The original user-supplied ZIP has two _C0 masters (436,692 and 437,310 instructions) and one _C1 runtime called JIN Moves (5,113 instructions). Both masters load automatically once and the cached runtime executes every ~16ms, with PPSSPP Cheats globally OFF.

The original creator's guide requests "refresh interval 16" in the Cheats tab and says "Plugins required" without naming them. This PRX avoids relying on CWCheats, but unspecified third-party plugins may still be required; functionality is not Android verified. The creator's archive does not explicitly document an input; the user reports that d/f+1+2 triggers Rage Art in this mod.

Install files under PSP/PLUGINS/Tekken6JinKiwiYu: plugin.ini, Tekken6JinKiwiYu.prx, jin_master.j8c, tekken8_master.j8c, jin_moves.j8c. The two masters are applied in the original order after the timer gate detects a live round. Unsupported modes fail closed, retaining original Jin without changes.

Do not enable optional _C0 groups for music, hazards, forced Rage, or Heat Meter (the latter requires Alboss textures). The full resource bytes must come from the user's ZIP, not from the old ULUS10466 (3).ini.

For a test, turn off Cheats globally, enable only this KiwiYu plugin and the original ultrawide, cold boot, and enter a normal timed fight with Jin P1. Check KiwiYu.log for "both KiwiYu masters initialized (2 signatures)" and "first runtime pass completed". If a crash occurs, stop and share the log. Do not rerun the known crash repeatedly.

This test build is not verified stable, cannot guarantee Rage Art execution, and the 7 MB master initialization can cause a battle-start pause.

### Original source integrity

`tools/build_kiwiyu_resource.py` requires exact verified source INI checksum and preserves the joined instruction line 799754 as two commands. No third-party source data was committed. [Comparison and hashes](../research/jin-lifecycle/experiment-009-kiwiyu-jin-10.md).

## v0.2 activation diagnostics + optional cheat pack

v0.1 was reported by the user to look like default Jin; it has **not** been confirmed to initialize the converted moveset on Android. The v0.2 PRX adds raw battle timer samples and the count of changed first-pass runtime memory writes. The first match should be tested with PPSSPP Cheats OFF and **no optional camera presets enabled**. If moves stay unchanged, obtain `PSP/PLUGINS/Tekken6JinKiwiYu/KiwiYu.log` and check whether it logged `ROUND TIMER ACTIVE`, `both KiwiYu masters initialized`, and `first runtime changed write operations`.

The user-specific ZIP additionally includes `PSP/Cheats/ULUS10466.ini` with all four original KiwiYu optional groups plus all four **exact original ultrawide camera presets**, all disabled (`_C0`). The file **overwrites any existing cheat INI upon extraction**. Back up and merge existing custom cheats before installing. Optional cheats require enabling global Cheats; **do not re-enable original JIN Moves / master cheats alongside the PRX**. Heat Meter requires Alboss textures. Choose at most one camera preset and be aware that KiwiYu runtime also writes the camera instruction at `0x0895350C` (17 original instructions), so Rage Art camera effects may conflict. [Audit](../research/jin-lifecycle/experiment-010-kiwiyu-v02-options-and-activation.md).
