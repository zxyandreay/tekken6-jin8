# Roadmap

**Status: ACTIVE RESEARCH — not yet implemented**

## Objective

Make Tekken 8-inspired Jin moves and Rage Art load automatically for Tekken 6 USA (ULUS10466) under PPSSPP, with **no manual master-cheat toggling** and no regressions in the separately installed ultrawide plugin.

## Architecture hypothesis

A small, independent `Tekken6Jin8.prx` will:
1. Verify supported game build and lifecycle state.
2. Wait for Jin's real moveset structures to be allocated.
3. Stage a versioned external data pack into validated memory.
4. Establish or re-establish custom move tables/pointers only when proven safe.
5. Implement necessary persistent behavior now performed by the Jin CWCheat.
6. Detect transitions, prevent duplicate application, and restore only changes it owns.

This is a **design target**, not a demonstrated plugin. A short-term cheat interpreter may be used to establish behavioral parity first; a native table-aware loader is a subsequent optimization.

## Milestones and gates

| Phase | Work | Evidence required |
| --- | --- | --- |
| 0. Baseline | Clean Jin, ultrawide-only, original Jin cheat, combined baseline | Snapshot matrix, PPSSPP version/config, video or move observations |
| 1. Cheat analysis | Decode conditional/write/pointer operations, master regions, persistent writes | Deterministic parser tests, range manifest, no unbounded writes |
| 2. Lifecycle | Identify character/moveset initialization and memory resets | Repeated read-only snapshots and narrow debugger trace |
| 3. Tiny proof | Auto-activate **one verified move** | Repeated cold boot / next-match passes with identical behavior |
| 4. Loader | External packed data + runtime operations | Memory equivalence checks vs source mod |
| 5. Rage Art | Resolve camera, animation, reaction, state machine | Full successful and unsuccessful activation paths |
| 6. Release | Compatibility, save state, performance, recovery tests | Full regression matrix and installation steps |

## Early success criteria

- Stock `ULUS10466` is recognized and wrong games are refused.
- With other characters selected, no custom Jin data is applied.
- A test move operates without opening or toggling Cheats in the current match.
- Reload/next match does not retain stale pointers or crash.
- Independent `tekken6-ultrawide` v1.2.1 works, including replacement fonts/textures and fast-forward.
- A Rage Art test returns camera/game state to normal after successful and interrupted sequences.

## Risk register

| Risk | Mitigation |
| --- | --- |
| Master has ~511K records | Convert into compact staged blocks; do not continuously replay |
| Runtime conditional semantics unclear | Capture and test each opcode group before porting |
| Memory size/ownership | Detect validity; verify required expanded memory; bound ranges and pointer dereferences |
| Shared camera setting | Define ownership and restore policy before writing |
| PRX footprint affects replacement textures | Keep ultrawide PRX unchanged; audit loader allocation/mapping |
| JIT instruction replacement hides stock values | Use `memory.read` with `replacements:false` for code inspection |
| Source asset redistribution | Keep source payload, ISO, EBOOT, and extracted assets out of repository |

## Deliberately out of scope (first version)

Other regional game builds, gameplay networking guarantees, exact Tekken 8 engine behavior, and full support for arbitrary character mods.

## October 10, 2026 — implementation checkpoint

A **standalone experimental PSP PRX** has now been written, compiled in PSPDEV/PSPSDK CI and packaged with a manifest requesting `memory = 80`. It loads two binary streams converted locally from the exact source INI and attempts automatic master initialization plus periodic Jin runtime execution. This initial build remains **untested in real PPSSPP**; passing CI is a compiler/build check only. The official `tekken6-ultrawide` code, binary and release are unchanged. The next stage is to test a cold-boot fight with both Jin CWCheats disabled, verify `Jin8.log`, custom move availability, ultrawide aspect/HUD, replacement fonts/textures and fast-forward, then debug only observed implementation failures.

## Incident 001 status — v0.1 rejected

User reported a **Bad Execution Address** crash and loss of replacement custom font textures. The v0.1 mapped PRX size **0x6A08** greatly exceeded the accepted ultrawide PRX footprint **0x0EB0** and employed global JIT/cache invalidation in runtime loops. v0.2 reduces mapped size to **0x1DD8**, uses local code-cache invalidation and includes an opt-in no-write `Jin8.safe` diagnostic. v0.2 is compiled only; it must **not** be marked accepted until device validates Jin behavior, ultrawide preservation, text replacement, and no crashes. See [incident report](../research/jin-lifecycle/incident-001-android-crash-fonts.md).

## Incident 002 — v0.2 rejected; lightweight native CWCheat backend v0.3

The user confirmed v0.2 reached the main menu and character selection but lost HD font/text replacements; on Jin fight entry it crashed with Bad Execution Address `PC=0x448DC65D`, `RA=0x08A53338`. The asynchronous source-cheat interpreter and mapped PRX size are plausible contributing risks, not proven exact root causes. A guarded v0.3 version of the large interpreter compiled but mapped `0x3354` bytes and was **rejected without distribution** due to the previously established texture-size risk.

The recommended next test is now `plugin-lite/`: mapped PRX size `0x0434` bytes, marker only, full original master/runtime code installed with conditional `_C1` CWCheat gates processed by PPSSPP's native cheat scheduler. Original ultrawide PRX and manifest not modified. [Full incident and ABI details](../research/jin-lifecycle/incident-002-v02-crash-lite-v03.md). Device acceptance pending.

## Experiment 006 — user rejects v0.3 cheat-gate compromise; standalone optimization v0.4

The user reported partial custom Jin moves with stutters and still-missing custom font textures in v0.3. Crucially, v0.3 requires PPSSPP Cheats enabled, which violates the full automation objective. The new preferred build is **v0.4 standalone**, with a one-time protected master replay and a RAM-cached 2,458-record runtime (no gameplay I/O, no repeated giant master-group gates). Source and instructions are preserved; hot execution interval ~20ms. Compiles in PSPSDK, **not yet Android-validated**. Font compatibility deferred. See [Experiment 006](../research/jin-lifecycle/experiment-006-v04-standalone-optimization.md).

## Incident 003 (Oct 10) — before-fight v0.4 crash

v0.4 bad execution address `PC=0x10`, RA `0x0893F4D8`, crash before fight; screenshot showed Cheats enabled despite v0.4 standalone requirement. v0.3's auto `_C1` master/runtime may remain in the shared INI and must be disabled for an uncontaminated next run. The v0.4 fight-ready predicate was also too weak. v0.4.1 adds current-move pointer validation and ~2-second stable fight condition. PSPSDK compilation passed; **Android success is not claimed**. See [incident 003](../research/jin-lifecycle/incident-003-v04-prematch-null-jump.md).

## Incident 004 — v0.4.1 crash despite 2-second guard

User's attached log ends at `user_main resumed` after master initialization, with screenshot `PC=0x10`, `RA=0x0889691C`, Cheats=Y. No proof that the later runtime didn't execute because v0.4.1 did not log its tick. Master parser source contains only types 0,2,E; audit details in [incident 004](../research/jin-lifecycle/incident-004-v041-master-resume-crash.md). v0.4.2 adds old v0.3 cheat gate / preinstalled payload conflict detection, first-runtime boundary logging, and ships same unaltered third-party payload. Compilation passed; no promise of Android stability. First priority is **one clean test with PPSSPP Cheats OFF**.

## Incident 005 — v0.4.2 finishes first runtime pass, invalid jump persists

Confirmed v0.4.2 log through `first runtime pass completed`, followed by Android PC `0x00000010` / RA `0x088C7B50`, `Cheats: Y` status. PPSSPP-native CWCheat invalidates host JIT for writes across memory, while our PRX only covered the first 2 MiB. v0.4.3 collects and flushes all changed master/runtime ranges, including memcopy destinations; compiles with PSPSDK. Next test requires Cheats OFF; optional master-only diagnostic marker isolates master vs runtime. [Incident 005](../research/jin-lifecycle/incident-005-v042-jit-coverage.md).

## Incident 006 — v0.4.3 fails; stop speculative build cycle

Android crash remains `PC=0x10`, new RA `0x08896988` even after batched extended RAM JIT invalidation, master signature checks, and first completed runtime pass. One controlled **Cheats OFF + master-only marker** test in the existing build is needed before further engineering. The EBOOT return-site instruction and source opcode mix are documented in [Incident 006](../research/jin-lifecycle/incident-006-v043-repeat-null-jump.md). Do not add arbitrary delays or claim a fix.

## Incident 007 — v0.4.3 master-only / Cheats-OFF still crashes

User turned PPSSPP Cheats completely OFF, requested `Jin8.masteronly`, and still reproduced `PC=0x10`, RA `0x088E78EC` before fight. Old v0.3 CWCheat conflict is not necessary for this run; Jin runtime is not expected to run but actual marker processing requires confirmation via **fresh `Jin8.log`** (not present in accessible attachments). Do not iterate the crashing full build or alter official ultrawide. See [Incident 007](../research/jin-lifecycle/incident-007-master-only-cheats-off.md).

### Incident 007 evidence update — log confirmed

The exact master-only log has now been received and establishes completed master initialization and `user_main resumed` without Jin runtime execution. Global Cheats were OFF. Next effort: battle-ready-state detection plus native master write/condition parity; do not release another speculative full PRX or modify official ultrawide. [Full log and source audit](../research/jin-lifecycle/incident-007-master-only-cheats-off.md).

## Incident 008 / Experiment 007 — v0.5 guarded after real battle timer starts

After the verified Cheats-OFF master-only crash, v0.5 requires **documented round-timer address `0x090FBE4C` to decrease** repeatedly before master initialization. This replaces relying entirely on Jin's early-loaded move pointers and a 2-second settle. No confirmation = no memory writes (fail-closed). PSPDEV compile and standalone timer unit tests pass, but Android timing, stability, and smoothness remain unverified. The full user-original master/runtime data remain unchanged. The ultrawide official PRX remains untouched. [Experiment / artifacts](../research/jin-lifecycle/incident-008-v05-battle-clock.md).

## KiwiYu experiment 009 — isolated Rage Art candidate

An alternate user's supplied Jin 1.0 by KiwiYu source has 874,002 total master instructions and 5,113 runtime instructions (versus the prior 511,607 + 2,458), and needs type-D conditions and two distinct masters. Its guide specifies a 16ms refresh interval and unnamed plugins. The user's reported Rage Art input is d/f+1+2, still unverified in the PRX. Independent v0.1 plugin built successfully; require real-world stable round, confirmed master/runtime log, Rage Art animation and frame pacing before promotion. Source converter preserves one joined _L line as two records. Official ultrawide unaffected. [Experiment 009](../research/jin-lifecycle/experiment-009-kiwiyu-jin-10.md).

## Experiment 010 — KiwiYu v0.2 actual write diagnostic + optional packs

v0.1 reported visually stock despite build success; without KiwiYu.log cannot know whether gate, master or runtime failed. Added first/periodic round timer logging and first runtime changed-write count in v0.2. User-owned optional source has four distinct _C0 groups, combined with four exact ultrawide camera presets all _C0 (off). Found 17 Kiwi runtime writes to same camera address 0x0895350C, so camera cheat cinematic compatibility not guaranteed. Core PRX independent of Cheats; optional groups need Cheats ON. No source master bytes published; ultrawide unchanged. [Experiment 010](../research/jin-lifecycle/experiment-010-kiwiyu-v02-options-and-activation.md).

## Incident 011 — inactive KiwiYu v0.2 fixed by v0.3 in-fight input+move gate

The user-supplied v0.2 log reports `0x090FBE4C == 0` during real fight. Old timer-only gate refused all master writes, so stock Jin was correct observed result. v0.3 adds a verified in-fight P1 input + current-move transition gate, with ~300ms follow-up and no early pointer-only injection. PRX compilation and host tests pass, **Android unknown**. Preserve original user data, optional cheats/camera entries, and official ultrawide. Next test: enter fight, wait ~2sec, press Square, check `PLAYER ACTION CONFIRMED` and master/runtime log; stop after crash. [Report](../research/jin-lifecycle/incident-011-kiwiyu-zero-timer-v03-action-gate.md).
