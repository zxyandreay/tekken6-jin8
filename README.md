# Tekken 6 Jin8 — Automatic Jin Moveset (Experimental)

**Latest test build: v0.3 lightweight gate + PPSSPP-native CWCheat.** Builds successfully, but Android gameplay, custom texture/fonts and Rage Art have **not yet been verified**. Earlier v0.1/v0.2 interpreter PRXs are **rejected** due to Android crashes and missing custom font replacements.

## v0.3: independent of official ultrawide

- `plugin-lite/Tekken6Jin8Gate.prx` is a **separate, tiny PRX** (mapped memory `0x0434` = 1,076 bytes).
- It sets only a one-time activation marker in expanded PSP RAM. The **original master/runtime cheat logic executes inside PPSSPP's built-in cheat scheduler**, not from another PSP game thread.
- `tools/build_auto_cheats.py` converts the locally owned original Jin8 master and runtime cheat into one-time-auto master and post-initialization runtime groups.
- This is still a **plugin + game-specific cheat-data package**. PPSSPP's Cheats feature must remain enabled, but users no longer toggle master/Jin manually.
- The official `Tekken6Ultrawide` PRX, code, manifest, and release are **not modified**. v0.3 carries the four original optional ultrawide camera cheat presets in the generated `ULUS10466.ini`, all disabled by default.
- The full third-party moveset data remains **outside public Git history**. Use your own original source cheat file to generate it locally.

## Build and install

See [v0.3 instructions](plugin-lite/INSTALL.md) and [incident/fix investigation](research/jin-lifecycle/incident-002-v02-crash-lite-v03.md).

```powershell
python tools/build_auto_cheats.py "path/to/ULUS10466 (3).ini" --out build/auto/ULUS10466.ini
```

Build `plugin-lite/` with PSPSDK (`make -C plugin-lite`) or download the [lightweight gate GitHub Actions artifact](https://github.com/zxyandreay/tekken6-jin8/actions/workflows/build-jin8-lite.yml).

The resulting `plugin-lite` PRX and automatically gated `ULUS10466.ini` are both needed. Back up any existing game cheat file before replacement/merge. Remove v0.1/v0.2 Jin8 PRX before testing. Keep your official ultrawide release unchanged.

## Research, troubleshooting, and compatibility

- [Incident 001 — v0.1 crash and custom-font regression](research/jin-lifecycle/incident-001-android-crash-fonts.md)
- [Incident 002 — v0.2 fight-start crash and v0.3 architecture](research/jin-lifecycle/incident-002-v02-crash-lite-v03.md)
- [Runtime research experiments](research/jin-lifecycle/README.md)
- [Ultrawide coexistence requirements](docs/architecture/ultrawide-compatibility.md)
- [Read-only debugger probes](docs/development/remote-debugging.md)
- [Standalone original v0.1/v0.2 PRX source (archived/rejected)](plugin/src/main.c)

**Scope:** Tekken 6 USA `ULUS10466`, PPSSPP `1.20.4` test target. No promise that the original third-party conversion includes every Tekken 8 move, full Rage Art, or flawless original cinematic effects. Cold boot for comparisons; do not reenable the original `(m)Tekken 8` / `(1)jin` cheats alongside new auto groups.
