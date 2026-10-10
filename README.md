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

## KiwiYu Jin 1.0 — separate experimental source (v0.1)

The user supplied a different `Jin 1.0 by KiwiYu.zip` including two masters and one `JIN Moves` runtime, with a user-confirmed source Rage Art input `d/f+1+2`. A separate **`plugin-kiwiyu/`** PRX has been added, requiring **no native PPSSPP Cheats** and preserving the official ultrawide plugin unchanged. It supports the source's type-D conditions, uses the existing battle-clock safety gate and ~16ms runtime cadence, and has verified CI compilation; **Android success is still untested**. This does not replace the established v0.5 until accepted. [Full source comparison and test instructions](research/jin-lifecycle/experiment-009-kiwiyu-jin-10.md).

## KiwiYu v0.2 — optional groups + actual activation telemetry

A user reported that KiwiYu v0.1 looked like stock Jin; **v0.1 was never verified to apply the moveset**. v0.2 logs first/periodic battle-clock values and a count of changed runtime memory writes so a silent safe-mode fallback can be distinguished from actual mod activation. A private user-specific package includes the four original **disabled optional KiwiYu groups** plus the four **disabled ultrawide camera presets**, separately from the standalone PRX core. All optional entries remain `_C0`. Enabling optional cheats needs the global PPSSPP Cheats toggle, but the automatic PRX core does not. Kiwi runtime writes camera address `0x0895350C` too, so optional camera presets may conflict with Rage Art cinematics. **Gameplay unverified; do not call this a fix for stock Jin.** [Experiment 010](research/jin-lifecycle/experiment-010-kiwiyu-v02-options-and-activation.md).

## KiwiYu v0.3 — fixed confirmed inactive v0.2 activation gate

User's `KiwiYu.log` **proved** `round timer = 0` and `NO MASTER WRITES`; previous stock Jin was not a partially functioning conversion—the two masters never ran. v0.3 waits for a *real P1 button plus active move-ID transition* after a stable round start, independently of unsupported timer address, before applying both KiwiYu masters. No global Cheats required for core. Byte-identical original mod data and all eight `_C0` optional cheats/camera presets preserved; Ultradwide official repo unchanged. PSPDEV and host gate tests passed. **Android safe initialization, actual move replacement and Rage Art remain unverified.** [Incident 011](research/jin-lifecycle/incident-011-kiwiyu-zero-timer-v03-action-gate.md).
