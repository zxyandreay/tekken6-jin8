# Incident 014 — Uppercut move 0x02AB: actual read-only remote debugger capture

**Date:** 2026-10-10. **Status:** Move-localized freeze confirmed by the user and read-only PPSSPP debugger. Root cause **not yet determined**, no PRX modification made. v2 targeted read-only probe prepared and locally protocol-tested.

## Artifacts received

- `KiwiYuProbe-20261010-225832.zip`
- `KiwiYuProbe-20261010-225832.jsonl`
- `KiwiYuProbe-20261010-225832.summary.json`
- Explicit user clarification: "**the point where i marked a move is where the game freezed at the uppercut moment of jin**".

The combined trace runs **81.75 seconds** with **182 P1 state samples**, 23 CPU status requests, seven successful GPU statistic readings, sixteen GPU-stat failures, eighteen sampling gaps and one manual marker. PPSSPP **v1.20.4**, game **ULUS10466**.

## Strongest observations

| Time from beginning | Event |
| ---: | --- |
| 0.28s | P1 Jin with current move `0x8001`, move pointer `0x087AD474`, 60 FPS |
| 3.67–7.34s | Normal observable transitions through `0x01A6`, `0x01D0`, `0x01D6` |
| **7.561s** | Last move transition: `0x01D0 → 0x02AB`, move pointer **`0x087B555C`**, input `0x00FB0400` |
| 7.73s onward | P1 input returns neutral; move ID **stays 0x02AB**, current move pointer and moveset base remain the same |
| **10.963s** | First `gpu.stats.get` failure, after `0x02AB` began; subsequent GPU timeouts recur |
| 31.037s | Successful GPU statistic response `fps.actual = 10`, target ~31.58 |
| **51.705s** | User marker `jin is about to uppercut p2` with last move `0x02AB` |
| 54.982s | Successful GPU statistic response `fps.actual = 0`, target 0 |
| 81.19s | Final P1 sample remains `0x02AB`, pointer `0x087B555C`; no recorded later transition |

**Nuance:** The user marker occurs 44 seconds *after* the recorded transition into `0x02AB`, so the trace itself does not reveal the exact frame at which the uppercut first froze. The user clarified that the marker labels the visually frozen uppercut. It would be incorrect to claim the initial transition at 7.56s is conclusively the instant of freezing.

PPSSPP's official [GPU stats WebSocket handler](https://github.com/hrydgard/ppsspp/blob/master/Core/Debugger/WebSocket/GPUStatsSubscriber.cpp) queues `gpu.stats.get` and replies **after the next frame flip**. A request timeout during the stalled `0x02AB` episode is therefore consistent with no next frame occurring during that request window. This is strong evidence of a genuine display/gameplay stall, though timeouts alone cannot prove an emulated CPU deadlock.

## Observed unchanged values

- Character ID Jin = `0x00000009` and moveset base = `0x08754400`.
- Camera `0x0895350C`: **always** `0x3C013F80` in all 23 samples.
- Interpolation candidate `0x08D6F270`: **always** `0x0000002B`.
- Large-copy sentinel `0x08C07D24`: **always** zero, not `0x12345678`.
- `0x090FBE4C` timer: zero (as established in earlier crashes).
- Two Kiwi Yu master signature values remain intact: `0x00008001`, `0x08B98190`.

**Interpretation:** Does not support recurring camera/interpolation toggles or the blocked large-copy sentinel as an *observed* trigger during this capture. It does NOT rule out other move-specific script states or pointer bugs.

## Debugger limitations

- The old v1 probe extracts only six fields from the 0x540-byte P1 state block, not animation-progress counters, so an apparently constant move ID is not proof that every other part of game state stopped advancing.
- All 23 old `cpu.status` entries show `emulated_us = null` and `clock_hz = null` on this user's Android build; its `pc` hint is explicitly unreliable under JIT. **Do not infer game CPU hang from the PC values alone.**
- Reported GPU FPS 0 may be stale during a full freeze; however asynchronous `gpu.stats.get` request timeouts after the same move transition strengthen the no-frame hypothesis.
- The v0.4 game PRX's own log records one master initialization, while the user's previous probe confirms 12 runtime writes/16 KB invalidation in a later sampled cycle. Neither report provides the actual move-script progression while the uppercut is stuck.

## Next experiment: targeted remote probe v2 (read-only)

- Private user-specific download: `KiwiYu-Uppercut-Remote-Debug-Probe-v2.zip` (11,256 bytes; SHA256 `47a601dc6d4cc5a8b21556ed706483a5f6188121596ec8a91ba08f4f16eabe80`).
- Contains `kiwiyu_remote_probe.py`, `RUN-PROBE-WINDOWS.bat`, `README.md`, and `tests/test_probe.py`; source not published to this GitHub repository, same as v1.
- Extra observation without writes: CRC and **byte-level difference count** of the whole 0x540 P1 state block, raw 1344-byte P1 snapshots on move transition and manual marker, 128-byte move descriptor reads/CRC at the current move pointer, additional Kiwi Yu referenced animation/phase addresses `0x08C08348`, `0x08C0839C`, `0x08C0864C`, `0x08C140B8`, large-copy condition candidate `0x09BDF2FC`, complete available `cpu.status` response, GPU reply latency and consecutive timeouts, accurate user marker input timestamps.
- 5 local automated tests passed against mock WebSocket server, including read-only event allowlist, memory decoding/diff, and end-to-end recording. **Actual v2 Android trace not yet received.**
- Test one uppercut episode with Cheats OFF, v0.4 PRX + unchanged official ultrawide. Mark the moment it visibly freezes as promptly as practical, then continue 10–15s. Upload resulting `KiwiYuProbe-*.zip` and the same session's `KiwiYu.log`.
- **Do not enable** `KiwiYu.allowlargecopies` marker or edit MIPS game memory to try to fix the freeze without evidence. PPSSPP remote debugger must stay on trusted private LAN and be disabled after testing.

## Next decision gate

Compare v2 before/after marker for broader actor-state changes. A frozen actor state with GPU flips stopping indicates a stalled game or emulation loop; actor-state counters advancing while move `0x02AB` remains active may indicate a special-move script that cannot exit, animation/asset transition, or deliberate cinematic behavior. Only then select a move-specific PRX fix. No claim of stable gameplay until a real phone test completes.
