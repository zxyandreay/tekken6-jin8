# Tekken 6 Jin8 experimental PRX installation

**Target:** PPSSPP / TEKKEN 6 USA (ULUS10466), game v1.00.

**Status:** Experimental build. Automatic injection has NOT yet been validated
in a live running emulator. Back up your save data and memory-stick files.

This is an **independent PSP plugin**, not a patch to Tekken6Ultrawide.
Do not modify or replace the official Tekken6Ultrawide PRX.

## Installation

1. Back up the user's existing PSP/Cheats/ULUS10466.ini and PPSSPP saves.
2. Disable both CWCheat groups "(m)Tekken 8" and "(1)jin".
   **Do not run them concurrently with the PRX**, as two writers can conflict.
3. Copy the separately built PRX and plugin.ini to
   PSP/PLUGINS/Tekken6Jin8/ inside PPSSPP's memory stick.
4. Generate master.j8c and runtime.j8c with:
   python tools/build_jin8_resource.py "PATH_TO_ORIGINAL/ULUS10466 (3).ini"
   Then copy them from build/resource/ to PSP/PLUGINS/Tekken6Jin8/.
5. The folder must contain plugin.ini, Tekken6Jin8.prx,
   master.j8c, runtime.j8c.
6. Enable the Jin8 plugin in PPSSPP's Plugin Manager; its own manifest
   requests memory=80. Leave the official Tekken6Ultrawide release **unchanged**
   and enabled if you want ultrawide.
7. Cold boot Tekken 6, select Jin as P1 and enter a fight. The loader
   attempts to initialize the master once for a valid Jin fight and
   automatically evaluates the runtime block. No cheat-menu toggling.
8. Inspect PSP/PLUGINS/Tekken6Jin8/Jin8.log to see loading status.

## Source and behavior constraints

- Replays the third-party cheat's original camera / SlowMo changes; this
  initial experimental build does **not** redesign or fix the source moves.
- Both plugins load independently and do not share binaries or project files.
- Master replay is intentionally not split into multiple frames; it may
  stall or fail. Watch the log and memory signatures before trusting it.
- Unknown cheat opcodes abort instead of silently producing partial data.
- Compatible only with the game version under investigation.
- PPSSPP save states created with either plugin enabled may preserve patches.
  Use cold boots for A/B tests.
- The third-party moveset content is not uploaded to the public source repo.
- The code does not disable or modify other plugins automatically.

## Report first test

Record whether PPSSPP boots, both modules appear in the debugger,
Jin8.log says "master initialized", and whether one custom Jin move
works with both CWCheats disabled. Test texture replacements separately.

## v0.2 crash regression and safe-mode diagnostic

The first PRX mapped `0x6A08` bytes, including a 20KB statically resident runtime cheat table, and invalidated the **entire instruction cache every 50ms**. In the ultrawide project a controlled memory-layout A/B established that enlarging a PRX from `0x0EB0` to `0x12D0` was *sufficient* to break some replacement texture identifiers. The v0.2 Jin8 build streams runtime data from its own resource file rather than keeping the 20KB table permanently mapped, uses a 128-line I/O buffer, and only invalidates the specific modified game-code address when the value changes. It reduces the thread stack as well. These reduce risks but **do not prove** the crash and texture regression fixed.

If PPSSPP still crashes, you can test an *identical binary and memory manifest without any cheat writes*: create an empty file `PSP/PLUGINS/Tekken6Jin8/Jin8.safe` then cold-boot. The log will say `SAFE MODE`. This control is essential to distinguish *module footprint / expanded memory* from *payload execution*, not a working moveset mode. Delete `Jin8.safe` afterwards to re-enable automation. Never enable source Jin CWCheats simultaneously.

## DEPRECATED / REJECTED — Important

**Do not install this old large-interpreter PRX.** Android v0.1/v0.2 builds were reported to crash or lose custom font textures. A guarded-but-larger interpreter variation was also rejected without device distribution. The new preferred experimental implementation is [Jin8 Lite Auto v0.3](../plugin-lite/INSTALL.md), using a small gate PRX plus PPSSPP's normal scheduled CWCheat interpreter.
