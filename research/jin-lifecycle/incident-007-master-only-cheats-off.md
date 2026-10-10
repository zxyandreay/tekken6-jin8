# Incident 007 — master-only crash with PPSSPP Cheats OFF (2026-10-10)

## Direct observations from the user's latest Android crash screenshot

- User states they disabled PPSSPP cheats globally and renamed `Jin8.masteronly.example` to exactly `Jin8.masteronly` before cold boot.
- The screenshot independently confirms **`Cheats: N, Plugins: Y`**. Therefore the earlier hypothesis that active v0.3 CWCheats were required for the crash is **excluded for this run**.
- `Bad Execution Address`, PC/Destination **`0x00000010`**, RA **`0x088E78EC`**, thread `user_main`, CPU core `JIT`. PPSSPP v1.20.4 Android arm64; TEKKEN 6 USA `ULUS10466` game CRC `ae9abfb0`.
- Battle still crashes; this test was performed with **master-only mode requested** (the Jin runtime is designed to be skipped in this mode).
- **New user-provided master-only log received and verified:** every line reproduced below. Earlier statement that it was inaccessible is superseded.

## Interpretation and limits

**Significant isolation:** cheats globally OFF yet crash with `PC=0x10`; outdated enabled CWCheats cannot be a necessary condition. **The received log now proves the MASTER-ONLY marker was recognized and both the master signature checks and game-thread resume completed; the Jin runtime did not execute in this build's master-only mode.** Consequently this crash does **not require** PPSSPP CWCheats or the standalone Jin runtime. The fault may be caused by the master pass, its application timing, or consequences of those changes, rather than an identified specific write.

**Root cause remains open.** Candidate categories: unsafe master-application timing, one or more incorrectly emulated source CWCheat conditions, game state corruption caused by full master install, JIT/timing issues or interaction with another loaded plugin. The log places the failure **after the master returned and `user_main` resumed**. A pair of correct sampled signatures does not prove the entire 511,607-record master was applied with correct conditions or timing. This observation is **not** sufficient to blame the official ultrawide plugin; do not edit its release.

## Additional static EBOOT call-site investigation

Using user's decrypted `ULUS10466_EBOOT(6).BIN` (ELF MIPS), guest `RA - 8 = 0x088E78E4` contains instruction word `0x0E2011A3`, encoding **JAL 0x0880468C**, followed by a delay-slot zero; guest `0x088E78EC` contains `0x24420030` (ADDIU v0,v0,0x30). The call target is game text and contains a guest-state pointer-access routine. This locates context but does not reveal why the later execution PC became `0x10` or what exact master record caused it.

## Exact user-provided master-only diagnostic log

```text
Tekken6Jin8 Standalone v0.4.3: batched full-range JIT invalidation
MASTER-ONLY diagnostic enabled
runtime cached in RAM; no gameplay I/O
protected module ranges: 0x00000002
Jin move table candidate found; waiting for battle stability
Jin battle stable for 2 seconds; checking master targets
master preflight passed
user_main suspended; beginning master initialization
master JIT invalidation completed
master initialized (2 signatures)
user_main resumed
```

The `runtime cached in RAM` line is a startup loading step, **not execution of the Jin runtime**. The master-only branch explicitly skips `execute_runtime()`. No `first runtime pass` line is present.

## Confirmed master source structure and parity limits

Parsed the original locally uploaded `ULUS10466 (3).ini` source for **all** `_L` entries in `(m)Tekken 8`:

| CWCheat opcode type | Record count |
| --- | ---: |
| Type 2 (32-bit direct write) | 511,199 |
| Type E (conditional skip) | 307 |
| Type 0 (width-dependent write; all `0x00000000 0x00000000`) | 101 |
| **Total** | **511,607** |

Only **three unique directly written addresses are below `0x09000000`**: `0x08E410D8` (99 records), `0x08E410E8` (99), and `0x08800000` (101 zero entries). The vast majority of direct records target `0x090...`, `0x091...`, `0x0A0...` and `0x0A1...` extended-memory payload ranges.

The PPSSPP native `CWCheatEngine::InterpretNextCwCheat` implementation handles master types 0, 2 and E with width-dependent writes and line-count conditional skips; the standalone interpreter uses broadly corresponding mechanisms, **but this source inspection is not a byte-by-byte parity test**. It also doesn't verify the active game's memory values at the moments each condition was evaluated.

**Timing hypothesis:** The master was historically enabled after entering a battle, but the standalone plugin treats a valid/current move pointer and two seconds of stable data as sufficient proof of battle readiness. The user's crash *while entering the battle* shows that this readiness heuristic may still be premature. That is a plausible failure mechanism, **not a confirmed root cause**.

## Engineering decision

Do **not** submit another speculative gameplay PRX or ask the user to keep reproducing this known crash. The next code effort should focus on:

1. Inspect the game-state transition to reliably distinguish character/stage preloading from **battle-ready gameplay**, beyond simply checking Jin ID/current move pointer.
2. Build an offline master-condition/write-trace parity harness against PPSSPP's native CWCheat semantics, using the original source and available known-working memory snapshots; identify any divergences before applying new guest writes.
3. Test a staged/late or otherwise safe automatic master installation only after that evidence, with fault recovery and a precise stage log. No PPSSPP Cheats dependency and no changes to the official ultrawide release.

**Status:** isolated to post-master-resume; original runtime and global Cheats no longer implicated as requirements. Root cause still unproven; HD custom-font mapping is lower priority; no new binary delivered.
