# Incident 002 — v0.2 fight-entry crash and decision to replace interpreter (2026-10-10)

**Observed on Android** Xiaomi `23049PCD8G` v35 (arm64), PPSSPP `1.20.4`, Tekken 6 USA `ULUS10466`, known-good game CRC `ae9abfb0`.

## User observations — confirmed, not inferred

- v0.2 loads **main menu and character select** without immediate crash.
- **Custom PPSSPP replacement font/text textures do not apply** with the experimental Jin8 plugin installed.
- **As Jin's match starts**, PPSSPP shows `Game crashed / Bad Execution Address`:
  - Destination = PC = **`0x448DC65D`** (odd address, outside supported code space).
  - RA = **`0x08A53338`**, CPU JIT, current thread `user_main`, attributes `0x800040FF`.
- v0.1 earlier crashed with PC `0xD9FF4A60`, RA `0x08B4CFB8`; both invalid PC values vary between runs.
- **No `Jin8.log`, crash call stack or PPSSPP module load-address map was supplied.** Neither the screenshots nor the differing RA values prove the particular write responsible.

## Specific concerns in the rejected v0.1–v0.2 engine

1. The first two PRXs **reimplemented CWCheat** within a separate PSP user thread, writing hundreds of thousands of values **while the game could continue running**. The original source mod was enabled through PPSSPP's host-side scheduled cheat interpreter. Thus concurrent partial installation is a strong plausible cause of invalid game pointers/jumps, but it is not conclusively established.
2. v0.2's mapped PRX `p_memsz=0x1DD8` (7,640 B), still exceeds the accepted ultrawide PRX's `0x0EB0` (3,760 B). The ultrawide project's earlier **memory-size-only A/B** caused custom texture/font replacements to stop matching after growing the mapped PRX from `0x0EB0` to `0x12D0`. See `zxyandreay/tekken6-ultrawide/research/autohud/archive/test-builds.md`.
3. The crash occurred at the expected master/runtime initialization boundary. No v0.2 safe-mode A/B has yet been provided, so we cannot attribute the entire font regression specifically to PRX footprint instead of imported cheats or game memory.

## Rejected intermediate implementation

A defensive `plugin/src/main.c` revision attempted protected-module overlap preflight and temporary `user_main` suspension. It compiled, but increased `p_memsz` to **`0x3354` = 13,140 B** (GitHub Actions `38032168316`, failed size check). Given the historical texture regression, **this was intentionally not packaged/recommended for device testing**. Its code remains in Git history for reference. Do not confuse it with the new lightweight v0.3 implementation.

## v0.3 — architecture pivot: minimal gate PRX + PPSSPP-native CWCheat

- Independent **`plugin-lite/`** module `Tekken6Jin8Gate.prx`, built using PSPSDK.
- Mapped ELF32 single-load resident size **`0x0434` = 1,076 bytes**, actual PRX file **2,574 B**.
- The tiny PRX modifies **only guest `0x0A1FFFC0`**, a marker after the original master's highest direct-write destination `0x0A1F0AB0`. It never writes Jin's moveset, EBOOT code, game camera, HUD or textures itself.
- Game readiness requires P1 Jin, a recognized moveset base prefix `0x086A/` `0x0875`, nonzero move ID and plausible current move pointer.
- On a valid new fight the PRX sets marker **1**; after the cheat completes, the native PPSSPP engine changes marker to **2**. On fight teardown it resets marker to 0. Timeout guards prevent persistent reapplication after failure.
- `tools/build_auto_cheats.py` converts the exact previously supplied original `ULUS10466 (3).ini` (SHA-256 `96541d37ad9ec8114c5f07feca63143d90cc0cdcda2218494cb19c970a8603d3`) to a **user-specific auto-enabled `ULUS10466.ini`**.
- The **511,607 original master** instruction pairs remain in original order; 135 E0 conditional gates split them at boundaries outside every original E skip interval (max section 3,800 physical lines). These gates skip the entire master unless marker==1.
- At the end of master, **both marker==1 and late signature `[0x0A1F0AB0] low16==0x7C0E`** are required to write marker=2.
- The **2,458 original Jin runtime** instruction pairs remain in original order, guarded as one E0 skip block of 2,458 records, only active when marker==2.
- Every imported master/runtime line and its order were checked against the original user-supplied source in an offline local verification, and the official ultrawide project's four optional camera cheats were kept as originally defined, default OFF.
- **All full cheat writes now run through PPSSPP's own CWCheat scheduler**. This addresses the async/secondary-engine risk while keeping an automatic plugin gate.
- No files changed in `zxyandreay/tekken6-ultrawide`. v0.3's own manifest requests `memory=80`, without modifying the user's official ultrawide installation.

## Artifact provenance

- Source PRX build run: [`38032665164`](https://github.com/zxyandreay/tekken6-jin8/actions/runs/38032665164) — success.
- New gate PRX SHA-256 `ab58e18bef32f749e3881ff391faf26b87a648c50299de54ec8fe4b25bf39f8b`, file 2,574 B, mapped 1,076 B.
- Generated gated cheat INI SHA-256 `aad80d8b63a337562fb0ba1d36fa470cd5b0e80d632949396e2c9f7971da82e3`, file 12,855,406 B.
- User-specific private experimental ZIP `Tekken6Jin8-v0.3-LiteAuto-ULUS10466.zip`: **3,278,178 bytes**, SHA-256 `8fa5ff5dcb3bd51e9127ac4ecb7be75d0d2b7ec89f10071f00212c6e2a2f0457`.
- **The third-party moveset payload is deliberately not committed or uploaded to this public GitHub repo**; only the code generator and non-payload findings are kept in Git.

## Test and acceptance boundaries

1. Completely remove/disable the **old** experimental `PSP/PLUGINS/Tekken6Jin8/` PRX; **do not** run both versions.
2. Back up `PSP/Cheats/ULUS10466.ini`. The v0.3 ZIP installs a user-specific **replacement shared game cheat file**, necessarily containing the two autoenabled groups. Preserve/merge any unrelated custom cheat settings separately. This does **not** replace/modify ultrawide's official PRX or plugin manifest.
3. Keep original ultrawide installed, enable Jin8 Auto gate and PPSSPP Cheats, cold-boot from game, not save state.
4. **First check HD font replacements on the main menu**, before master executes. Then attempt a Jin P1 fight and an identifiable custom move.
5. Report if font textures fail already before a match, if PPSSPP crashes, if custom moves remain stock, and if ultrawide HUD or texture/fast-forward regress. Observe sentinel via debugger if necessary; no new gameplay probe is required before the first device test.
6. If anything fails, disable Jin8 Lite plugin and restore backed-up cheat INI, cold boot. Leave ultrawide unchanged.

**Status:** build verified and structural conversion checked; **no user-device runtime validation** for v0.3. Do not call crash fixed, texture issue solved, Rage Art working, or coexistence proven until verified.
