# Incident 015 — Uppercut v2 debugger capture: game-state flatline and emulated-time starvation

**Date:** 2026-10-10. **No successful gameplay fix yet.** The user uploaded a second **read-only PPSSPP remote debugger trace**, including `KiwiYuProbe-20261010-231132.zip`, `.jsonl` and `.summary.json`. The target is Jin's uppercut freeze during the Kiwi Yu standalone v0.4 moveset, with original Ultrawide unchanged.

## Objective findings in the uploaded trace

- Game: PPSSPP **v1.20.4**, `ULUS10466`, P1 Jin `0x09`.
- Recording: **111.66 seconds**, 265 state samples, 35 CPU samples, 16 received GPU statistics, 18 GPU-stat timeouts, 19 poll gaps.
- At **t=24.275s**, Jin transitions `0x01D0 → 0x02AB`, pointer `0x087B555C` (after a normal series of moves). The 128-byte move descriptor CRC `9DB865D0` stays constant.
- P1 state stops changing shortly afterward: **150 consecutive samples from t=25.005 through 111.009s have exactly zero changed bytes across the full 1,344-byte P1 state block**, always CRC `0FB47997`. Last change happened at `t=24.875s`. P1 input is stuck at `0x00FD0200`, move ID `0x02AB`, move pointer `0x087B555C`.
- Initial frame stats are 38–60 FPS; after uppercut t=24.584s there is one result with 51 FPS; the first GPU response timeout occurs t=27.565s and repeats. A later response at t=50.486s reports 34 FPS but took ~924ms and is not proof of ongoing rendering at normal rate. Official PPSSPP `gpu.stats.get` replies after next display flip, so repeated pending timeouts corroborate a severe frame-delivery stall.
- User's precise marker: text `game is frozen at the uppercut moment of jin`, input timestamp **t=84.642s**, processed t=85.662s. The marker was pressed well after the **first observable state flatline at ~25s**; do not misplace freeze onset at 85s.
- Before move, CPU ticks rise from `20,627,429,398` at t=.393s to `27,016,312,632` at t=24.566s: average **264.3 million emulated ticks per wall second**. From t=29.704s to t=111.625s, only **2.216 million ticks per wall second**: ~**119-fold slower**. These are not CPU utilization statistics; consistent with emulated-time starvation.
- The running JIT `cpu.status.pc` frequently reports **`0x088C6160` / `0x088C6168`**, sometimes `0x08B57E88`, during the stall. **Running JIT PC is not reliable**, so do not claim it proves an infinite CPU loop.
- In the user's decrypted `ULUS10466_EBOOT(6).BIN`, `0x088C6160` encodes MIPS `0x1500FFEC`: **`bne $t0, $zero, 0x088C6114`**, a backward conditional branch. `0x088C618C` is another backward branch (`bne $a1, $s5, 0x088C6160`). We must capture accurate **paused registers, call stack and MIPS disassembly** to determine whether this is a genuine pathological loop and which operands sustain it; assigning root cause to a damaged pointer here would be unproven.
- Kiwi Yu's two master signature values are intact in later samples, as is a constant camera/interpolation value during the frozen period. No logged master replay or successful source 8.5 MiB copy occurs in this trace.
- Original `JIN Moves` group contains **no literal `02AB` source instructions**, so the specific move can still be affected indirectly through pointer walks, static master table data or conditions. Do not infer a safe single-command fix from this alone.

## Newly prepared targeted CPU snapshot tool

- `KiwiYu-Uppercut-CPU-Snapshot-v1.zip` (7,240 bytes), SHA-256 `6bcf45baca9602ed1bcc696c0783ee0f78655252dc16913049ee89b37bd2652d`. Four included files: Python script, Windows .bat launcher, README and tests. Delivered directly as a user-specific conversation artifact.
- This **is not the same type of fully read-only continuous recorder**: it briefly requests **CPU pause** (`cpu.stepping`) twice to get reliable values, then requests **resume** (`cpu.resume`) in a `finally` block. It does **not** write game memory, set breakpoints, change Cheats/input/speed or modify any plugin source. Avoid using it during normal gameplay.
- Before pause it validates game ID and current move `0x02AB`. It then records `cpu.getAllRegs`, `hle.thread.list`, `hle.backtrace`, `memory.disasm` at paused PC (plus at suspected game code loop), and post-resume status. Two snapshots separated by ~600ms, outputs `probe-results/KiwiYuCPU-*.zip`.
- Simulated PPSSPP WebSocket protocol tested with **3 Python unit tests**: LAN/private-IP validation, correct two CPU pause/snapshot/resume cycles with all source-only requests, and refusal to pause if current move != `0x02AB`. All passed. **User Android v1.20.4 hasn't run this snapshot yet**.
- Safety: Some real PPSSPP builds may fail to pause or resume automatically during an already frozen frame; user may need to manually Resume/Continue or restart. Remote debugger should run only on trusted LAN and be disabled afterward.

## Next action and acceptance

Keep KiwiYu v0.4, official Ultrawide and original optional cheat decisions intact, Cheats OFF. Reproduce one `0x02AB` uppercut freeze; while frozen, run the CPU snapshot .bat from Windows on same Wi-Fi. Upload `KiwiYuCPU-*.zip` and same-session `KiwiYu.log`. Inspect *paused* GPR operands at original executable loop, thread state/backtrace and nearby instructions. Determine whether stalled game loop, corrupted master pointer, or interpreter conditional mismatch is implicated before writing a **targeted** new PRX.

**No new gameplay build was produced by this investigation.** Avoid claiming stability or a complete bug fix.
