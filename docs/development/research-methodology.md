# Research methodology

**Status: CURRENT**

## Evidence hierarchy

1. Exact supplied binary facts and checksums.
2. Captured PPSSPP runtime memory/register evidence, labeled with emulator/build/config/phase.
3. Controlled A/B visual and gameplay behavior.
4. Published community knowledge and source-code hypotheses.

Do not silently promote (4) into (1) or (2).

## Repeatable investigation

1. Lock game ID + EBOOT hash, source cheat hash, ultrawide release hash, PPSSPP version and configuration.
2. Define a single question (e.g. is moveset pointer recreated at fight load?).
3. Capture a stock/no-plugin control.
4. Change **one variable** (master activated, runtime cheat active, or ultrawide enabled).
5. Capture the same game state and same character position if feasible.
6. Compare field values, pointers, memory-range checksums, and observed move behavior.
7. If necessary, stop in debugger at the **narrowest** discovered execution point; record PC/RA/SP and arguments before altering code.
8. Record conclusion as **ACCEPTED**, **REJECTED**, **NOT YET PROVEN**, or **SUPERSEDED**.
9. Repeat on next fight and after full game restart.

## Data minimization

Record bounded address windows, not indiscriminate RAM dumps. No full source game assets in git. Normalize source files into summaries and fingerprints where possible.

## Build discipline (future PRX)

Guard each hook with expected original instruction words. Handle MIPS delay slots, calling conventions, instruction/data cache invalidation, and relocation-safe PRX addressing. Use repeatable builds and exact SHA-256 before testing.
