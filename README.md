# Tekken6 Jin8 — Fully automatic standalone PPSSPP plugin

**Current experimental build:** v0.4.1 guarded standalone, independent of PPSSPP Cheats. Targets Tekken 6 USA `ULUS10466` with original Jin8 source data. Android gameplay stability and stutter reduction **remain to be validated by the user**.

## Goal and current build

- **No Cheats feature required.** Enable only the separate `Tekken6Jin8` PRX in the Plugin Manager; PPSSPP Cheats can be OFF.
- Automatic master data initialization once when Jin P1 enters a valid fight, rather than a permanent 511,607-record CWCheat group.
- Cache all 2,458 original Jin runtime records in RAM and execute without game-time storage reads.
- Keep the official ultrawide PRX, manifest and release untouched. Standalone Jin8 requests its own `memory=80`.
- Preserve source move conversions, with smooth gameplay as the immediate goal. Custom font replacement compatibility is lower priority and unresolved.

[Install the compiled v0.4 PRX](plugin-standalone/INSTALL.md) · [Optimization / incident history](research/jin-lifecycle/experiment-006-v04-standalone-optimization.md) · [PSPSDK build](https://github.com/zxyandreay/tekken6-jin8/actions/runs/38034521986).

The public build workflow contains the PRX/manifest only. You need to provide your own source cheat converted into `master.j8c` and `runtime.j8c` using `tools/build_jin8_resource.py`. The privately supplied conversation ZIP contains the fully assembled user-provided resources.

## Rejected experiments

- **v0.1/v0.2:** separate large interpreter caused reported bad execution address crashes and HD font mapping regressions.
- **v0.3:** tiny gate and PPSSPP-native auto CWCheats produced some custom moves but reported stuttering, still lacked HD font replacements and **requires Cheats enabled**. Therefore rejected as contrary to the automation requirement.
- **v0.4:** returns to truly standalone execution, but runs master safely once and caches the active runtime. This is an engineering experiment, not a confirmed stable release.

[Full research log](research/jin-lifecycle/README.md) · [Ultrawide compatibility](docs/architecture/ultrawide-compatibility.md)

## Latest crash checkpoint (Incident 003)

v0.4 crashed before the fight loaded with PC `0x00000010`, RA `0x0893F4D8`; its screenshot showed `Cheats: Y`. The older v0.3 zip installed two `_C1` auto cheat groups, so concurrent execution has **not been ruled out**. The new v0.4.1 standalone source requires a valid current-move pointer and a stable two-second fight-ready interval before initializing the original master. It still needs an Android test with **PPSSPP Cheats OFF**. See [incident investigation](research/jin-lifecycle/incident-003-v04-prematch-null-jump.md).

## Incident 004 — v0.4.1 PC=0x10 immediately following master resume

The supplied Jin8.log establishes successful master preflight, two expected memory signatures, and `user_main resumed` before the crash, while the screenshot still says `Cheats: Y`. Unchanged v0.3 auto `_C1` cheats may therefore still be active (not confirmed). The new v0.4.2 standalone guard refuses master writes when it sees the old auto-gate or already populated master signatures, and logs the first runtime pass. **It still requires testing with PPSSPP Cheats globally OFF and does not claim the crash is solved.** [Incident 004 analysis](research/jin-lifecycle/incident-004-v041-master-resume-crash.md).

## Incident 005 — v0.4.2 first runtime completes but PC=0x10 persists

The user supplied a crash PC `0x10`, RA `0x088C7B50`, and a Jin8.log proving `first runtime pass completed`. A PPSSPP source comparison found a concrete mismatch: our standalone PRX previously invalidated its JIT only for memory writes in `0x08800000..0x08A00000`, missing extended-RAM payload and memcopy destinations. v0.4.3 batches guest JIT invalidation across **all changed addresses**. This is **a compiled test fix, not yet an Android-verified crash resolution**. Keep Cheats globally OFF. See [Incident 005](research/jin-lifecycle/incident-005-v042-jit-coverage.md).
