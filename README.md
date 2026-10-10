# Tekken 6 Jin8 — Automatic standalone PPSSPP plugin

**Current experimental build: v0.5 battle-clock gate.** This project aims to run the Tekken 8-inspired Jin Kazama moveset in Tekken 6 USA (`ULUS10466`) **with PPSSPP Cheats OFF**, alongside the unchanged original `Tekken6Ultrawide` plugin.

## Status

- Master-only v0.4.3 crashed after master initialization with Cheats OFF; the user provided the exact diagnostic log. The original runtime is **not necessary** for that crash.
- v0.5 changes the injection trigger: a character ID and nonzero moveset pointer are not proof the battle is ready. The loader now requires **multiple observed decreases in the actual round timer** before master installation. It is designed to fail closed when the timer isn't recognized.
- The full original 511,607 master and 2,458 runtime source records remain unchanged; the standalone PRX caches runtime data in RAM and never requires a manual cheat toggle.
- The official ultrawide repository and binary remain unchanged. HD replacement font/textures remain lower priority.
- **PSPSDK build and timer-gate unit tests pass; v0.5 has NOT yet been tested on Android.** This is not a demonstrated crash fix or stutter-free release.

[Installation instructions](plugin-standalone/INSTALL.md) · [Incident 008 / v0.5 findings](research/jin-lifecycle/incident-008-v05-battle-clock.md) · [GitHub Actions build](https://github.com/zxyandreay/tekken6-jin8/actions/workflows/build-jin8-standalone.yml).

The public PRX artifact excludes the user-supplied master/runtime bytes. A private user-specific ZIP containing the full original converted resources was provided directly in the conversation. Alternatively use `tools/build_jin8_resource.py` on your own verified `ULUS10466 (3).ini`.

## Previous attempts and unresolved work

[v0.1/v0.2 crashes](research/jin-lifecycle/incident-001-android-crash-fonts.md) · [v0.3 gate and stutters](research/jin-lifecycle/incident-002-v02-crash-lite-v03.md) · [v0.4.3 master-only crash proof](research/jin-lifecycle/incident-007-master-only-cheats-off.md) · [Next diagnostics](docs/roadmap.md)
