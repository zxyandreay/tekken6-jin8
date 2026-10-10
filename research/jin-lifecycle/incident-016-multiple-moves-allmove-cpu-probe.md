# Incident 016 — Multiple different modified Jin moves freeze (2026-10-10)

**New user clarification:** Previous CPU snapshot did not reproduce a freeze because the user forgot to trigger it. Importantly, **freezes also happen with other Tekken 8 Jin moves; not exclusively the uppercut**. The earlier two v1/v2 remote traces proved `0x02AB` was affected but do not prove it is uniquely broken. Treat the issue as potentially common to move transition handling / static master data / runtime CWCheat parity / emulation.

## Existing observed facts

- v0.4 KiwiYu standalone with official ultrawide enabled, PPSSPP Cheats OFF; both masters load, the runtime actually executes, and only one master initialization was logged.
- The previous v2 uppercut trace showed P1 `0x02AB`, 150+ unchanged full state samples, GPU-stat timeouts and approximately 119-fold slower emulated ticks/second. A running-JIT PC hint around `0x088C6160` points to original-game backward loop, but **paused CPU registers have not yet been captured**. There is no safe basis to patch that code or delete original Jin animations.
- The latest user-provided `KiwiYuCPU-20261010-233836.zip` was **not a valid freeze capture**: the snapshot program saw P1 move `0x8001` and applied **no CPU pause**; accompanying `KiwiYu (3).log` by itself cannot attribute move freezes.
- The prior `KiwiYu-Uppercut-CPU-Snapshot-v1` was incorrectly restricted to move `0x02AB`, which is too narrow after the user's new clarification.

## New private diagnostic artifact

`KiwiYu-AllMoves-CPU-AutoSnapshot-v2.zip` (12,505 bytes, SHA256 `f0d4df3ddb0fa1adbeb85e7c5434513eeafd6463bc154e1d3c6dbc86353e4bc2`). Includes Windows launcher, Python 3 recorder, README and tests. Created within the active conversation, **not published to the GitHub repository**. Original Kiwi Yu source and official ultrawide PRX are unchanged.

The program uses PPSSPP's official WebSocket debugger in a private LAN, calls only `version`, `game.status`, `memory.read`, `cpu.status`, `cpu.stepping`, `cpu.resume`, `cpu.getAllRegs`, `hle.thread.list`, `hle.backtrace`, and `memory.disasm`. Unlike earlier read-only probes, CPU snapshots **briefly pause/resume** to get accurate registers.

**Auto detection** requires: Jin P1 ID 9, observed transition into **any** nonidle move (not merely `0x02AB`), 1344-byte P1 block byte-identical for ~3 seconds, emulated CPU tick rate below 12% of normal for **two** consecutive status readings. A manual `C` capture is possible after at least 1.5 sec of identical P1 block on any nonidle move, if ticks are unavailable. Idle 0x8001 / invalid 0x0000 is explicitly excluded from auto-trigger. This reduces accidental pauses during benign idle, stationary cinematics or loading.

Six local tests passed, including simulated PPSSPP WebSocket paused register capture/resume, a different simulated frozen move `0x023E`, no-trigger tests for still-running CPU and idle state, and offline replay of the user's actual v2 uppercut trace with auto detection at **t=35.224s**. **No actual new Android snapshot yet**.

## User acceptance next

1. Keep PPSSPP + KiwiYu v0.4 + official ultrawide unchanged; native Cheats OFF for isolated test.
2. Turn on PPSSPP remote debugger, use the same trusted Wi-Fi and port as previous probes.
3. Run **`RUN-ALL-MOVES-SNAPSHOT-WINDOWS.bat` before entering the fight**. Let it learn normal CPU progress before attempting any troublesome modified move.
4. Trigger whichever Tekken 8 modified move reproducibly freezes first; wait for automatic trigger, or press `C` during a frozen non-idle move if necessary. Do not pause PPSSPP manually.
5. Upload `probe-results/KiwiYuCPU-*.zip`, corresponding `KiwiYu.log`, and describe the button input / freeze point.
6. Compare paused PC, register operands, game thread backtrace **across multiple failing move IDs**. Same stopped loop or bad pointer across different moves would support a common failure; different backtraces may indicate several independent corrupt move routes.

**Do not ship a speculative new gameplay PRX claiming stability from a single frozen move.** Device validation and specific fault diagnosis are pending.
