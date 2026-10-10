# Incident 005 — v0.4.2 first runtime completed, repeat PC=0x10, and JIT coherency fix

**Report received 2026-10-10.** PPSSPP v1.20.4 on Xiaomi 23049PCD8G (Android arm64), Tekken 6 USA `ULUS10466` CRC `ae9abfb0`.

## Direct attached evidence

- Battle still crashes with **Bad Execution Address**, `PC / destination = 0x00000010`, `RA = 0x088C7B50`, `user_main` thread, JIT CPU.
- Status reads **`Cheats: Y; Plugins: Y`**. This confirms Cheats globally enabled in the crash-report status but **does not establish which named cheat groups are actually active**. The rejected v0.3 ZIP included enabled-by-default `_C1` master/runtime groups. A fully isolated Cheats-OFF result has not yet been supplied.
- User attached `Jin8 (1).log`, content:
```text
Tekken6Jin8 Standalone v0.4.2: fail-closed legacy conflict detection
runtime cached in RAM; no gameplay I/O
protected module ranges: 0x00000002
Jin move table candidate found; waiting for battle stability
Jin battle stable for 2 seconds; checking master targets
master preflight passed
user_main suspended; beginning master initialization
master initialized (2 signatures)
user_main resumed
first runtime pass beginning
first runtime pass completed
```
- Unlike v0.4.1, v0.4.2 **confirms one complete runtime pass**. It does NOT confirm every runtime modification was correct, that later passes didn't run, or that the runtime itself is the crash cause.
- The guard printed no `CONFLICT` line. This rules out its **two precise detection predicates during its check**, not all possible concurrent cheat activity.
- Offline decrypted EBOOT at `RA - 8 = 0x088C7B48` contains `0x0E231B5D`, a normal `jal` whose target is `0x088C6D74`. This locates the game call site but **does not prove** what corrupted the eventual execution target.

## Concrete v0.4.2 code defect — JIT invalidation misses injected memory

The rejected standalone loader `plugin-standalone/src/main.c` invoked `sceKernelIcacheInvalidateRange` *only* for direct writes in **`0x08800000 <= addr < 0x08A00000`**, although the original master writes many executable candidate instructions/payloads in `0x091...`, `0x09...` and `0x0A...` extended RAM. Its type-5 memcpy and type-6 nested copy paths also performed no JIT invalidation for the destination.

This differs concretely from PPSSPP's built-in `Core/CwCheat.cpp` interpreter: `CWCheatEngine::ExecuteOp` invokes `InvalidateICache(op.addr, op.sz)` for cheat writes, and `Memory::Memcpy` is handled with invalidation; PPSSPP's `sceKernelIcacheInvalidateRange` HLE ultimately calls `currentMIPS->InvalidateICacheRangeDeferred(addr, size)` and `Core_ReenterDispatcher()`. Sources:
- [PPSSPP CwCheat.cpp](https://github.com/hrydgard/ppsspp/blob/master/Core/CwCheat.cpp)
- [PPSSPP sceKernel.cpp](https://github.com/hrydgard/ppsspp/blob/master/Core/HLE/sceKernel.cpp)

This is a **verified mismatch**, with a plausible link to stale JIT execution and invalid branches, but there is **no PPSSPP Android memory/jit trace proving it was responsible for PC=0x10**. Do not label this a confirmed root-cause fix.

## v0.4.3 test implementation

- **Every changed direct write across valid expanded guest RAM** now marks the destination for guest-JIT invalidation, not only addresses inside the first 2 MiB of user RAM.
- Both type-5 `memmove` and type-6 pointer-copy `memmove` destinations now mark modified ranges, matching expected guest-JIT cache coherency responsibilities.
- To avoid hundreds of thousands of syscalls and unnecessary gameplay stalls, the **511,607-record master** records one high/low touched address span and triggers **one deferred JIT invalidation after initialization**.
- The small 2,458-record runtime stream collects up to 128 changed address ranges, coalescing nearby writes; changed ranges are invalidated at the end of each cached runtime pass. If more than 128 ranges are touched, it falls back to one bulk span.
- No old cheat file, game EBOOT, official ultrawide PRX, manifest, release, or original master/runtime records have been modified.
- Master-once, cached-runtime, protected PRX ranges and two-second battle-readiness gate preserved.
- The new module remains **fully standalone**: Cheats should be OFF, no manual activation.

## v0.4.3 build/packaging provenance

- Patch commit `7d299d6aa33e5e020d40dec1205cf2f341d28138`.
- PSPDEV GitHub build [38038296257](https://github.com/zxyandreay/tekken6-jin8/actions/runs/38038296257): **success**.
- PRX `Tekken6Jin8.prx`: 38,698 bytes, ELF single load mapped `0x8114` = 33,044 bytes, SHA-256 `3c64e9de6ebc6a85e3bf038f5057ab7569047293f7f6be812a0e82ad1de0fe05`.
- User-specific ZIP `Tekken6Jin8-v0.4.3-JITSync-Standalone-ULUS10466.zip`: 2,425,672 bytes, SHA-256 `7ee1a62ce7e7457e24bc3f41935da64a38a0bf8a1c36604e0bd3cbfa7735150d`.
- Source mod binary files unchanged: `master.j8c` SHA-256 `956b045210749885ad21da15c0c190f5d6db34fdd8fb998c517c5097fb4c37ca`; `runtime.j8c` SHA-256 `ace847db19328eca86b05412d9ee8f4cc1349ec34ea97f0630c7f570dc7a68aa`.
- Converted third-party data distributed only through conversation user-specific ZIP, **not public Git history**.

## Next device test (one clean attempt)

1. Back up saves and `PSP/Cheats/ULUS10466.ini`; confirm **PPSSPP Settings → System → Enable Cheats is globally OFF**, not merely the two named cheats disabled. Ensure old `Tekken6Jin8Gate` removed and no previous Jin8 PRX active.
2. Replace ONLY the Jin8 folder with v0.4.3. Do not alter the official ultrawide plugin.
3. Cold boot and try Jin P1 match. If crash persists, stop and supply the fresh crash screen and `Jin8.log`.
4. For one targeted discriminator **using this same build**, create an empty `PSP/PLUGINS/Tekken6Jin8/Jin8.masteronly` and cold boot: it loads the master but skips Jin runtime entirely. If master-only crashes, focus on the master stage/JIT; if master-only is stable, focus on runtime writes/concurrency. Delete the marker after this optional diagnostic.
5. HD font/texture compatibility remains lower priority; this build is *not yet accepted* as stable, smooth gameplay.

**Known limitations:** A batched broad master JIT invalidation may still incur one fight-start pause. Repeated runtime invalidations might affect frame pacing and require profiling if the first Android test gets past the crash. JIT validity and move behavior remain unverified on-device.
