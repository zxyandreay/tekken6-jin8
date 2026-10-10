# Experiment 006 — v0.3 stutter and standalone optimization (October 10, 2026)

**User requirement:** Fully automatic Tekken 8-inspired Jin Kazama moveset via an independent PSP plugin. **PPSSPP Cheats must be OFF/unused.** Preserve the unchanged official `Tekken6Ultrawide` release. Prioritize smooth gameplay above custom HD text/font texture replacement.

## User-observed result on Lite Auto v0.3

- Jin's source-inspired moves **appear to work in part**, but gameplay **stutters**.
- HD custom font/text replacements still do not apply.
- The v0.3 design was fundamentally unacceptable for the goal: it used two auto-enabled CWCheat groups and required PPSSPP's Cheats feature ON. **v0.3 is rejected even if its cheats work.**
- No PPSSPP profiler/frame-time trace was provided. Do **not** assert the exact cause of the stutter.

## PPSSPP cheat-scheduler source review

Reviewed current [PPSSPP `Core/CwCheat.cpp`](https://github.com/hrydgard/ppsspp/blob/master/Core/CwCheat.cpp):

- `hleCheat` reschedules a cheat event at the configured refresh period and invokes `cheatEngine->Run()`.
- `Run()` processes all enabled cheat groups on every event; `IfEqual` skips `op.ifTypes.skip` physical lines when false.
- In the v0.3 generated INI the master data was cut into ~135 sections, each gated to skip up to 3800 physical records after initialization. **Clarification:** Those gates skip most of the 511,607 records; it is not correct to claim that all 511,607 writes execute every refresh. But repeated 135 condition checks, memory reads and JIT invalidation requests, plus the 2,458-line runtime group, still consume scheduler work. The one-time master may cause a separate startup spike.
- These implementation costs are **plausible contributors**, not established as the user's precise frame-pacing defect. Source cinematic slow-motion rules remain another possible contributor.

## v0.4 standalone implementation — no cheats

- New independent `plugin-standalone/` PRX. It requests `memory=80` in its own manifest; leaves `Tekken6Ultrawide` untouched.
- **Completely independent of PPSSPP Cheats.** `PSP/Cheats/ULUS10466.ini` is not installed or modified by v0.4; disable Cheats and remove/disable the old Jin8 Gate PRX.
- Loads the exact original user-provided master/runtime resource streams (`J8C1`, 511,607 + 2,458 original instruction records), preserving original record order and binary hashes.
- Reads and verifies the small runtime resource **once at plugin startup**, allocates and caches all 2,458 records in RAM. No repeated `sceIoOpen/read/close` on active fight ticks.
- Scans master direct-write targets against detected loaded PRX module segments and checks lifecycle conditions; aborts without applying if safety checks fail.
- At Jin P1 fight readiness, suspends game `user_main`, runs the massive master **once per fight**, verifies beginning and late source signatures, resumes `user_main`. That may cause a **single initialization pause**, distinct from repeating gameplay stutter.
- Executes only the cached small runtime on ~20 ms ticks. No recurring master scans or master writes while stable in the same Jin fight, and no per-tick suspend/resume of `user_main`.
- Fixed the original interpreter's CWCheat pointer-chain handling to include optional second walk step. Preserved targeted instruction-cache invalidation on changed game code.
- Resident ELF mapped size: **30,512 bytes**. This is much larger than the accepted ultrawide PRX and remains a risk for HD texture/font mapping. The user explicitly assigned font compatibility a lower priority for this test.
- Original ultrawide PRX/release/manifest/commit unchanged. Coexistence is a design constraint but **not yet verified on the user's Android device**.

## Build / artifact

- First standalone source commit `b55f7d33e47e21b18503ac91a39ecbdffc3c83c7`.
- PSPSDK build [GitHub Actions 38034521986](https://github.com/zxyandreay/tekken6-jin8/actions/runs/38034521986): **success**.
- PRX file SHA-256 `fafd724d5a68e145fd4436c24445ac7eecf3f75c9d5336ceecee19f0b18f219a`, size 36,162 bytes, mapped `0x7730` (=30,512 B).
- User-specific ZIP: `Tekken6Jin8-v0.4-Standalone-NoCheats-ULUS10466.zip`, 2,424,267 bytes, SHA-256 `dd65cc2174bd3987deae410ea9c6a45b2761efcb300da273feddaf1216fb1716`.
- Original validated data: `master.j8c` 4,092,864 B, SHA-256 `956b045210749885ad21da15c0c190f5d6db34fdd8fb998c517c5097fb4c37ca`; `runtime.j8c` 19,672 B, SHA-256 `ace847db19328eca86b05412d9ee8f4cc1349ec34ea97f0630c7f570dc7a68aa`.
- Distributed ZIP not committed publicly because source mod's redistribution permission is unverified. Generator and PRX source are public.

## Acceptance / remaining uncertainties

1. Fully close PPSSPP. Back up and remove both old `PSP/PLUGINS/Tekken6Jin8/` and `PSP/PLUGINS/Tekken6Jin8Gate/`. Install only new standalone Jin8 folder, keep ultrawide official release unchanged.
2. Disable PPSSPP Cheats **entirely**. The original INI can remain backed up; v0.4 doesn't need it enabled.
3. Cold-boot, select Jin P1, start a match and report whether custom moves load automatically, whether first fight starts/crashes, whether frame pacing is smooth after first load, and whether ultrawide works.
4. If it crashes or fails closed, supply `PSP/PLUGINS/Tekken6Jin8/Jin8.log` and crash screen, then disable v0.4.
5. This is **a test build**, not demonstrated stable gameplay. The earlier independent interpreter crashes make real-device verification mandatory before labeling it fixed.
6. If runtime ticks still stutter, profile actual frame times and source slowmo conditions before reducing tick rate; slower ticks can miss brief move conditions.

**Font/text replacements are explicitly deferred, not marked solved.**
