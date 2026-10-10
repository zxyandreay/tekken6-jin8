# Tekken 6 Jin8

**Status: compiled experimental automatic PRX — real-device PPSSPP verification pending.** This repository contains a standalone PSP plugin to reproduce the user's existing Tekken 8-inspired Jin Kazama cheat **without manually toggling CWCheats**. It targets Tekken 6 USA (\`ULUS10466\`) on PPSSPP.

## Install the independent Jin8 plugin

1. Download the **Tekken6Jin8-Experimental-PRX** artifact from the [build workflow](https://github.com/zxyandreay/tekken6-jin8/actions/workflows/build-jin8-experimental.yml). That public artifact contains the compiled PRX and plugin.ini, **not** copyrighted third-party cheat data.
2. Copy the artifact's \`PSP/PLUGINS/Tekken6Jin8/\` folder to your PPSSPP Memory Stick directory.
3. Convert your local, original \`ULUS10466 (3).ini\` into the required separate resources:

\`\`\`powershell
python tools/build_jin8_resource.py "C:\path\to\ULUS10466 (3).ini"
\`\`\`

4. Copy \`build/resource/master.j8c\` and \`build/resource/runtime.j8c\` next to \`Tekken6Jin8.prx\` and \`plugin.ini\`. Verify all four files are under \`PSP/PLUGINS/Tekken6Jin8/\`.
5. **Disable both (m)Tekken 8 and (1)jin CWCheats.** Do not run two cheat engines against the same game RAM.
6. Enable Jin8 in PPSSPP's Plugin Manager, fully restart PPSSPP, and select Jin as P1. Examine \`PSP/PLUGINS/Tekken6Jin8/Jin8.log\` for errors.
7. Keep the **official Tekken6Ultrawide** PRX installed independently and unchanged if you want ultrawide. The Jin8 manifest itself requests \`memory = 80\`, without requiring any modification to ultrawide.

The initial PRX reproduces existing cheat semantics (including cinematic SlowMo/camera effects). It is **not yet verified in live PPSSPP**, and does not promise a complete Tekken 8 moveset or Rage Art.

[Detailed install and testing notes](plugin/INSTALL.md) · [Plugin source](plugin/src/main.c) · [Build workflow](.github/workflows/build-jin8-experimental.yml) · [Experimental research](research/README.md)

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

## October 10 — master initialization evidence

[Experiment 002 deep-capture report](research/jin-lifecycle/experiment-002-results.md) confirms direct-write signatures near the beginning and far end of the original master are present when modded Jin is active. The first deep stock/mod captures differed in ultrawide plugin loading, so they are not a controlled A/B. Next use the safe [11-signature probe](tools/probes/verify-master-payload.mjs) with a matched game/plugin configuration, and keep both output JSON files locally in ignored `captures/`.

## Experiment 004 — runtime pointer writes

Three of eight tested Jin runtime pointer destinations matched source-defined values in the modded fight, with the same three remaining matched across 20 samples. Stock Jin had no matches. The source explicitly supports both observed moveset pointer prefixes `0x086A` and `0x0875`. See [Experiment 004](research/jin-lifecycle/experiment-004-results.md). For move/condition correlation, use the **read-only** [move watch](tools/probes/watch-jin-move-conditions.mjs) instead of replaying the giant master repeatedly.

## Experiment 005 — cinematic source code confirmed active in capture

The user's attack watch observed a brief **speed-mode 3 / zoom 16.0 / camera-rate 0.5** state matching the original Jin runtime's `SlowMo v2.3 Master` lines, while a separate attempted-slowmo watch recorded no such state. The nominal idle capture was confounded by an attacking CPU. See [Experiment 005 findings](research/jin-lifecycle/experiment-005-results.md) and the safer [focused event monitor](tools/probes/watch-jin-cinematics.mjs).
