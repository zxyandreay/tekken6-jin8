# Independence and compatibility contract

**Status: STATIC ANALYSIS COMPLETE; DEVICE VALIDATION PENDING.**

Reference: [zxyandreay/tekken6-ultrawide](https://github.com/zxyandreay/tekken6-ultrawide) — documented accepted release **v1.2.1 / OPT-EXP4A**, PRX SHA-256:
`6a537691e758dabc912910b9cfb1758e9ea9b5a2554242a43ff87e281c32a8e8`.

## Rules

1. Ship a separate Jin8 PRX (provisional `PSP/PLUGINS/Tekken6Jin8/`). Do not merge or enlarge the ultrawide PRX.
2. Do not touch ultrawide's 3D aspect instruction pairs, renderer hooks, HUD patch sites, or binary footprint.
3. Preserve normal replacement texture/font loading and PPSSPP fast-forward. The ultrawide research linked enlarged PRX mappings to regressions in those behaviors; don't infer that a new module has zero impact.
4. Do not overwrite `PSP/Cheats/ULUS10466.ini` on installation. Optional camera presets and user cheats may already be present.
5. Treat camera code at `0x0895350C` as **shared state**; audit every associated write and restoration, especially during Rage Art.
6. New writes must have verified expected original values and ownership; reject unexpected memory state instead of forcing a patch.
7. Test Jin8 **alone**, ultrawide **alone**, both together, and clean stock on Windows and Android where possible.

## Known implementation details from reference repository

- Automatic aspect correction: four instruction pairs in `plugin/src/main.c`.
- Gameplay HUD uses owner-specific hooks for HP, timer, labels, round markers, and fonts.
- Accepted PRX maps one `PT_LOAD` with `p_filesz=p_memsz=0x0EB0` (its ELF file is larger); do not conflate disk file size and mapped payload.
- Camera cheats use `0x1015350C` addressing (host-space equivalent `0x0895350C`).
- The accepted release PRX has additional binary-optimized HUD hooks **not fully represented in `main.c`**. Refer to `docs/architecture/` and `research/renderer-optimization/findings/final-architecture.md` in ultrawide.

## Observed vs unproven

- **Observed statically**: overlapping camera target; distinct four aspect instruction pairs.
- **Not proven yet**: absence of all memory conflicts, multi-plugin allocation safety, startup order, and Rage Art camera restore.
- **Release gate**: no regressions in custom font/texture replacement, HUD, camera presets, fast-forward, or normal Jin move execution.

Upstream reference is read-only. Reproducing techniques in this repository does not justify copying third-party assets or bypassing their licenses.

## Incident 001: mapped PRX allocation may alter texture hashes

Ultrawide accepted footprint is one `PT_LOAD` with `p_filesz=p_memsz=0x0EB0`; its historical test changed *only* the resident mapped size to `0x12D0` and reproduced missing replacement font/texture selections. The separate Jin8 v0.1 had `p_memsz=0x6A08`; v0.2 trims this to `0x1DD8`, still larger than the proven-safe ultrawide footprint. Treat coexistence as *untested* until verified on-device. Do not edit ultrawide official release. See [Jin8 incident](../../research/jin-lifecycle/incident-001-android-crash-fonts.md).
