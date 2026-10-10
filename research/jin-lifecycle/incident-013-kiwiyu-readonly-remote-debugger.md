# Incident 013 — Read-only remote debugger probe for KiwiYu v0.4 freezes

**Status:** local probe utility built, packaged, and tested against a fake PPSSPP WebSocket debugger; **the phone has not been connected or captured yet**. Source and tests are provided inside the user-specific conversation artifact `KiwiYu-Remote-Debug-Probe-v1.zip` (not in this GitHub source tree).

## What is confirmed from the user's KiwiYu (2).log

- `KiwiYu Jin 1.0 standalone v0.4: precise JIT pages + legacy copy guard`.
- Smooth camera mode and blocking of 8.5 MiB snapshot copies were enabled by default.
- P1 real move/input activity qualified initialization at move `0x01DF`, input `0x00FB0404`.
- Both master resources initialized once (`total master initializations = 0x1`), then runtime executed.
- First runtime pass: `0x2C1=705` changed writes, `0x4A000 = 303104` invalidated JIT bytes across `0x32=50` batches.
- At runtime cycle `0x100=256`: `0x0C=12` changed writes, `0x4000=16384` invalidated bytes across `0x04=4` batches.
- These snapshots do NOT identify an offending move, opcode, or stalled system; author did not include timing probes. No evidence of a repeat master install in this log.
- User still experiences freezes in certain moves. We should now trace those specific transitions rather than continue removing random KiwiYu commands.

## Why the remote probe is appropriate

PPSSPP has a built-in **JSON/WebSocket debugger** on the Remote ISO/local-server port, path `/debugger`, subprotocol `debugger.ppsspp.org`, accessible when **Settings > Tools > Developer Tools > Allow remote debugger** is enabled. Official upstream: [WebSocket protocol](https://github.com/hrydgard/ppsspp/blob/master/docs/WebSocketDebugger.md), [Developer Tools](https://www.ppsspp.org/docs/development/developer-tools/).

A Windows-side Python 3 recorder on the **same private Wi-Fi network** connects with `websockets>=12,<17`. It only sends `version`, `game.status`, `memory.read`, `memory.read_u32`, `cpu.status`, `gpu.stats.get`. It **does not** set breakpoints, step, change game speed, inject buttons, write memory, or toggle cheat options. It rejects public IP addresses and stores all data locally.

Default sampling ~5 Hz reads a contiguous 0x540-byte P1 game-state block, extracting:
- `0x08C078B4` P1 character ID; `0x08C078E0` moveset base; `0x08C07964` current move pointer; `0x08C07A18` move ID;
- `0x08C07D24` memory-snapshot sentinel field; `0x08C07D3C` P1 input.
- Every eight state samples it reads `0x0895350C` camera, `0x08D6F270` interpolation, `0x090FBE4C` timer (historically zero), and both Kiwi master signature fields `0x0B000000`, `0x0A200000`.
- Also `cpu.status` emulated microseconds/wall time plus `gpu.stats.get` actual FPS and frame timing statistics. **CPU PC is documented as inaccurate while JIT is running; never infer the stuck MIPS instruction from this hint.**
- User can mark the moment of a freeze by typing a short label in Windows console. The utility writes line-delimited JSON records with timestamps, a summary, and a ZIP trace suitable for attachment.
- On a stalled debugger read, timeouts are recorded so a freeze does not silently disappear.

## Delivered local experiment

- ZIP: `KiwiYu-Remote-Debug-Probe-v1.zip`; 9932 bytes, SHA-256 `e702a1bb72471e0477176390d094787969d98192355d40899affefdb684d1874`.
- Included `kiwiyu_remote_probe.py`, `RUN-PROBE-WINDOWS.bat`, `README.md`, `tests/test_probe.py`.
- Four local tests passed with Python 3.13 and websockets 16.0, including a fake PPSSPP local WebSocket server, game ID check, mocked memory samples, move transition, CPU/GPU queries, persisted archive, the public-IP guard, and read-only event whitelist.
- **Cannot directly connect to or control the user's phone from this chat.** The Windows script runs on the user's computer and saves the trace locally for upload.
- The existing v0.4 PRX and official ultrawide repository are **not modified**. No new gameplay build until we see correlation between move/freezes and the runtime.

## Exact next steps

1. Phone: leave KiwiYu v0.4 and official ultrawide plugin as-is; for clean first test turn native PPSSPP Cheats OFF. Settings > Tools > Developer Tools > Allow remote debugger = ON. Under Settings > Networking select a known Local Server Port such as 8080; note the phone's Wi-Fi private IPv4.
2. PC (same Wi-Fi): unzip the probe ZIP, install Python 3 if necessary, and run `RUN-PROBE-WINDOWS.bat`. Enter phone IPv4 and matching port. The batch launcher installs Python `websockets`.
3. Launch Tekken 6 USA first; the probe requires game `ULUS10466`. Play Jin as normal, reproduce one troublesome move, type `freeze [move input]` and Enter in PC console, and keep recording for several seconds.
4. Press Ctrl+C to finish early if desired. Send generated `probe-results/KiwiYuProbe-*.zip` **plus latest** `PSP/PLUGINS/Tekken6JinKiwiYu/KiwiYu.log` and specify which move froze.
5. Diagnose whether frames/FPS stall while emulated microseconds advance, whether move pointer oscillates, whether a zero/placeholder pointer appears, whether camera/interpolation changes, and whether the failure is reproducible for the same move ID.
6. Only after live data choose a targeted fix, e.g. specific pointer semantics parity vs PPSSPP CwCheat.cpp, offending source segment (including 8.5MiB copy/sentinel dependencies), or renderer/JIT invalidations. **Do not claim this read-only probe itself fixes freezes.**

## Safety / privacy

This is a powerful debugger endpoint: do not port-forward it to the public Internet, keep it on trusted local Wi-Fi and turn it off when finished. The probe only logs a small list of game fields; no ISO or full memory dump. Review the trace before sharing.
