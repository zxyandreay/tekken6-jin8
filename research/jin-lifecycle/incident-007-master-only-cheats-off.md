# Incident 007 — master-only crash with PPSSPP Cheats OFF (2026-10-10)

## Direct observations from the user's latest Android crash screenshot

- User states they disabled PPSSPP cheats globally and renamed `Jin8.masteronly.example` to exactly `Jin8.masteronly` before cold boot.
- The screenshot independently confirms **`Cheats: N, Plugins: Y`**. Therefore the earlier hypothesis that active v0.3 CWCheats were required for the crash is **excluded for this run**.
- `Bad Execution Address`, PC/Destination **`0x00000010`**, RA **`0x088E78EC`**, thread `user_main`, CPU core `JIT`. PPSSPP v1.20.4 Android arm64; TEKKEN 6 USA `ULUS10466` game CRC `ae9abfb0`.
- Battle still crashes; this test was performed with **master-only mode requested** (the Jin runtime is designed to be skipped in this mode).
- The user subsequently said 'here's the log with cheats off and Jin8.masteronly', but **the new `Jin8.log` was not among the accessible conversation attachments** at the time this report was written. The latest visible log was older `Jin8 (2).log` from the previous full-runtime v0.4.3 test. **Do not treat that older log as the master-only log.**

## Interpretation and limits

**Significant isolation:** cheats globally OFF yet crash with `PC=0x10`; outdated enabled CWCheats cannot be a necessary condition. Since the user reports the master-only marker was present, the runtime-cheat execution likely is also not necessary. However, `Jin8.log` must show `MASTER-ONLY diagnostic enabled` and `master initialized (2 signatures)` / `user_main resumed` to confirm *which* stages executed before the fault.

**Root cause remains open.** Candidate categories: unsafe master-application timing, one or more incorrectly emulated source CWCheat conditions, game state corruption caused by full master install, JIT/timing issues or interaction with another loaded plugin. The log is necessary to distinguish a crash before master preflight, during master, and after it resumes. This observation is **not** sufficient to blame the official ultrawide plugin; do not edit its release.

## Additional static EBOOT call-site investigation

Using user's decrypted `ULUS10466_EBOOT(6).BIN` (ELF MIPS), guest `RA - 8 = 0x088E78E4` contains instruction word `0x0E2011A3`, encoding **JAL 0x0880468C**, followed by a delay-slot zero; guest `0x088E78EC` contains `0x24420030` (ADDIU v0,v0,0x30). The call target is game text and contains a guest-state pointer-access routine. This locates context but does not reveal why the later execution PC became `0x10` or what exact master record caused it.

## Safety disposition

Do **not** ask the user to rerun the crashing full loader, and do not declare v0.4.3 stable. A new speculative build isn't justified without knowing the last message in the current `Jin8.log`. First request and read the **fresh master-only** log, verify marker detection, and then design a narrow, instrumented stage probe or fix from the evidence. Preserve the goals: 100% automatic standalone PRX, no global cheats, unchanged official ultrawide, smooth gameplay primary, HD-font mapping deferred.

**Action:** user should upload `PSP/PLUGINS/Tekken6Jin8/Jin8.log` from this `Cheats:N + Jin8.masteronly` run.
