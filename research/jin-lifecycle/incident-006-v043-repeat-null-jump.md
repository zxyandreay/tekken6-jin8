# Incident 006 — v0.4.3 JIT synchronization did not fix PC=0x10 (2026-10-10)

**Status: OPEN. No claim of fixed crash. Do not publish another speculative automatic-loader build before the master-only isolation test.**

## New user-supplied evidence

The user again reports a Tekken 6 USA `ULUS10466` crash while starting battle on PPSSPP v1.20.4 Android (Xiaomi 23049PCD8G, ARM64). Attached purple crash screenshot shows:
- `Bad Execution Address`, destination/PC **`0x00000010`**.
- Return address **`0x08896988`**, on `user_main`, JIT CPU.
- `Cheats: Y, Plugins: Y`. This is a feature-status report; it does **not** prove which individual CWCheat groups are active. There has been **no confirmed Cheats-OFF run** yet despite the standalone plugin requiring no cheats.

Attached `Jin8 (2).log` (exact content):

```text
Tekken6Jin8 Standalone v0.4.3: batched full-range JIT invalidation
runtime cached in RAM; no gameplay I/O
protected module ranges: 0x00000002
Jin move table candidate found; waiting for battle stability
Jin battle stable for 2 seconds; checking master targets
master preflight passed
user_main suspended; beginning master initialization
master JIT invalidation completed
master initialized (2 signatures)
user_main resumed
first runtime pass beginning
first runtime pass completed
```

Thus both sampled master signatures and the first runtime invocation completed. The new JIT invalidation path also returned successfully. This does **not** prove every master/runtime record was correct or that only one runtime tick executed before the crash.

## Offline original EBOOT inspection

The decrypted, user-supplied `ULUS10466_EBOOT(6).BIN` is ELF32 MIPS little endian; load file offset `0x1018` corresponds to guest `0x08804018`.
The instruction at `RA-8 = 0x08896980` is `0x0E25C64C` = MIPS `JAL 0x08971930`; `0x08896984` is its delay slot, and return address `0x08896988` is ordinary game code. This identifies the most recent visible call site, **not** the actual bad indirect target nor the causal cheat write. A return/jump to `0x10` is consistent with dereferencing an uninitialized/invalid function pointer, but that remains a hypothesis.

## Source audit following v0.4.3 failure

Cross-checked the original user-supplied `ULUS10466 (3).ini` group composition against PPSSPP `Core/CwCheat.cpp`.
- Master has exactly **511,607** records: 511,199 direct 32-bit type-2, 307 type-E conditional skip, and 101 type-0 lines. The 101 type-0 master records are `_L 0x00000000 0x00000000`. These are not type-6 pointer continuations in master; runtime has numerous pointer continuations.
- Runtime has **2,458** records, including **194** pointer commands (type 6) and many conditional skips (type E). These are not all guaranteed to execute, and the first runtime pass may include state-dependent writes.
- PPSSPP's original type-E decoding and type-0 value-dependent width are consistent with the v0.4.3 interpreter's broad approach. There are still differences in safe address validation, JIT read invalidation, temporal scheduling, and pointer handling that need actual memory-output parity testing before attributing the crash to any one opcode.
- The v0.4.3 PRX synchronized the JIT cache over expanded-memory write ranges. The same `PC=0x10` crash **still occurred**, so broader JIT invalidation **alone is insufficient**.
- The 2-second battle-settle guard and module protection also did not prevent the fault in v0.4.1–v0.4.3.

## Exactly one next discriminating test (no new binary required)

Use the **same v0.4.3 package** and its existing optional `Jin8.masteronly.example` marker, to isolate the master pass from Jin runtime execution.

1. Completely close PPSSPP.
2. **Set PPSSPP Settings > System > Enable cheats OFF globally**, not just old groups, to rule out the v0.3 auto `_C1` master/runtime still present in the shared `ULUS10466.ini`. Verify the toggle state before cold boot.
3. In `PSP/PLUGINS/Tekken6Jin8/`, rename `Jin8.masteronly.example` to precisely `Jin8.masteronly` (zero-byte file). Leave v0.4.3 master/runtime files otherwise unchanged.
4. Keep the official ultrawide PRX and its manifest untouched; do not enable any old Jin8 Gate or manual mod.
5. Cold boot **once**, select Jin P1, attempt to enter battle. If it crashes, stop and send screenshot + `Jin8.log`; if it remains stable, note that there will not be complete Jin8 gameplay without the runtime, and send the log anyway.
6. Remove `Jin8.masteronly` after test; do not reattempt the crashing full mode until a targeted engineering change has been made.

**Interpretation:** If master-only crashes, the issue is upstream in the master pass, PRX layout, game loading/JIT or confounding cheats, *not dependent on a Jin runtime pass*. If master-only survives with Cheats OFF, focus on conditional runtime pointers, write timing and scheduler parity. Neither result alone proves absolute cause.

## Prioritization

Fully independent Jin8 PRX, zero manual cheats, official ultrawide untouched, smooth gameplay; font/texture replacements low priority as requested. Do not call v0.4.3 stable or promote it as a successful gameplay build. No new build until a specific defect is identified or the master-only control provides actionable evidence.
