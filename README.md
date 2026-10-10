# Tekken 6 Jin8

**Research-stage project** to automate a **Tekken 8-inspired Jin Kazama moveset and Rage Art** in **Tekken 6 USA (`ULUS10466`) on PPSSPP**.

> **Status: ACTIVE RESEARCH.** No working automatic moveset plugin is released or demonstrated yet. This repository currently holds the evidence, research plan, compatibility contract, and read-only PPSSPP debugger probes needed to build one.

## Goal

Selecting Jin should be enough. Users should not have to enter a match, open the Cheats menu, rapidly enable a huge master code, and disable it again. The eventual implementation should load custom moveset data at a verified point in the match lifecycle, maintain the necessary runtime behavior, and restore or invalidate it safely as game state changes.

The first milestone is **reproducible observations**, not a speculative PRX.

## Getting started: remote debugging

Requirements: **Tekken 6 USA (`ULUS10466`)**, **PPSSPP with remote debugger**, and **Node.js 22+** on the computer running the probes. Python 3.10+ is optional for source-cheat inventory. These scripts do not use npm dependencies.

1. In PPSSPP, enable **Settings → Tools → Developer Tools → Allow remote debugger**, and launch the USA game.
2. Find the debugger host/port shown by PPSSPP or its log. If the probes run on the *same* Windows computer, use `127.0.0.1:<port>`; from another device on the network use your PC's LAN IP and the reported port. Do not expose the debugger to the internet.
3. Run from this repo root:

```powershell
node tools/probes/ppsspp-smoke.mjs --endpoint 127.0.0.1:39100
node tools/probes/capture-jin-state.mjs --endpoint 127.0.0.1:39100 --label stock-jin-battle
```

**The port above is an example, not a known port for your PPSSPP installation.** Both probes accept `ws://HOST:PORT/debugger` as well.

4. Save each capture at the designated stage (see [first experiment](research/jin-lifecycle/README.md)). Compare two snapshots:

```powershell
node tools/probes/compare-jin-states.mjs captures/stock-jin-battle.json captures/mod-jin-battle.json
```

**No game-memory writes, breakpoints, cheats, or PRX installation are performed by these probes.** Captures are automatically saved under ignored `captures/`. They may contain memory values and emulator configuration; inspect before sharing.

## Research scope

- [Project plan and acceptance criteria](docs/roadmap.md)
- [Documents index](docs/README.md)
- [Memory map and evidence confidence](docs/architecture/memory-map.md)
- [Compatibility with tekken6-ultrawide v1.2.1](docs/architecture/ultrawide-compatibility.md)
- [PPSSPP remote debugger workflow](docs/development/remote-debugging.md)
- [First coordinated capture matrix](research/jin-lifecycle/README.md)
- [Known sources and credits](docs/references.md)

## Design direction (provisional)

A separate `Tekken6Jin8.prx` plus external data pack, not a modification to the ultrawide PRX. First reproduce the original CWCheat's initialization and ongoing runtime effects under controlled tests. Only then replace its understood operations with a purpose-built loader. The existing `tekken6-ultrawide` project must remain an independent plugin.

## Source data policy

Do **not** commit or distribute a decrypted Tekken 6 EBOOT, a game ISO, extracted commercial assets, or the third-party full cheat payload without appropriate rights. Document hashes, structure, and independently measured behavior instead. Third-party code is cited, not imported into this repo. No license is asserted for unlicensed external assets.

## Supported target

Research is currently scoped to `ULUS10466` and PPSSPP. Addresses and behavior may differ by region, game version, emulator configuration, and game state.

## Related project

- [Tekken 6 Ultrawide](https://github.com/zxyandreay/tekken6-ultrawide) — independent automatic aspect and gameplay HUD correction, with its own validated v1.2.1 release.
