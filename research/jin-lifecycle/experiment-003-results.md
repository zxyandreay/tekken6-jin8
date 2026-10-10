# Experiment 003 — matched Jin master activation + runtime-link hypothesis

**Status: MASTER SAMPLED DATA ACCEPTED; RUNTIME POINTER TARGETS NOT YET TESTED.**

Five user-supplied captures received October 10, 2026. Target: Tekken 6 USA `ULUS10466` v1.00, PPSSPP `v1.20.4`.

## Artifact identity (raw files kept local, not committed)

| Name | SHA-256 |
| --- | --- |
| `stock-jin-matched-deep.json` | `7de16b7983ab9b01665d5533f0c320655babdc91cd0df5e1b6c0f3751c4814d6` |
| `mod-jin-matched-deep.json` | `3d2b33daf86cc86401d3033cb01f7aa48f7829803e99fb85a35628c448ba7b51` |
| `master-stock-matched.json` | `3178c1762a6a2a341425f3fdb32e34b20f4bdc22e09f70ff2dc965f453c6cfa9` |
| `master-mod-matched.json` | `74722223a02c65a34f25a0cd979a64da5e53873b30bb4f99f55824d2991ebb45` |
| Reattached `mod-jin-battle-deep(1).json` | `182f956145c8741d8accbba849a5a028e47ab0ebe5e2860e8d2b596d5c1a663b` (duplicate of previous Experiment 002 mod snapshot) |

## Matched control and definitive sampled outcome

- Both new deep captures contain `Tekken6Ultrawide`; four aspect-pair sites patched identically to `0x3C01400E / 0x34214165`. This removes the **known module-presence confound** in Experiment 002, but does not prove identical cheat/camera/game state.
- Both have expanded-memory target `0x0A1F0AB0` readable. The stock capture has 16 zero bytes and the mod capture starts with `0e7cdf00` (= `0x00DF7C0E`).
- The 11 distributed direct-write signatures: **stock 0/11 match, 11/11 readable and zero; mod 11/11 match, 0 read errors**.
- First master target `0x09110000`: stock zeros, modified starts with `9081b9089081b908e400110942080000`.
- Master is **present at all sampled addresses** but the probe cannot certify execution of all 511,607 records, pointer linkage, Rage Art, or frame-perfect operation.

## Current move and static header

- Character ID `0x09` and moveset pointer `0x08754400` in both.
- 256 bytes at `0x08754400` unchanged (SHA-256 `5b3803ae8bc868d53ba83804f88918d74cd070e1ac822a5667195fddf4525654`).
- Stock `currentMoveID=0x8007`, current pointer `0x087A7C74`; mod `currentMoveID=0x01A6`, current pointer `0x087AFBA4`; mod incoming ID `0x01AC`. These are **different fight phases**, not an isolated mod difference.
- Input state differs (`0x00FF0000` vs `0x00FD0200`).

## Camera nuance: original read method could expose JIT/emuhack replacements

- Stock `shared_camera_word` was `0x682CE4C8`; mod `0x3C013F80`.
- This address is a **code instruction site**, but the earlier capture used `memory.read_u32`, unlike the aspect code probes, which already used `memory.read` with `replacements:false`.
- The user-supplied stock EBOOT has raw `0x3C013F80` at `0x0895350C`. Thus `0x682CE4C8` **may be a PPSSPP JIT/emuhack replacement, not a genuine camera patch**; we cannot conclude until re-reading both via `memory.read` with replacements disabled.
- The capture script has been corrected to use that read mode. Do not compare its new raw value naively against old snapshots' replacement-sensitive field.

## Source-analysis breakthrough: runtime pointer commands target deep offsets

The original `(1)jin` group contains many CWCheat `0x6` pointer commands at **`0x604078E0`**, using a follow-on line **`0x00020001 <offset>`**. This command dereferences the 32-bit word at `0x08C078E0` (in these runs `0x08754400`) and writes at the derived pointer-plus-offset. CWCheat pointer command interpretation is also described in [Sadamitsu's Tekken 6 research](https://www.sadamitsu.ru/) and the [CWCheat code reference](https://datacrystal.tcrf.net/wiki/CwCheat).

Example:

```text
_L 0x604078E0 0x0909CE00
_L 0x00020001 0x0002C250
```

In this captured game state, the possible destination is **`0x08754400 + 0x0002C250 = 0x08780650`** — **far beyond the first 256 bytes** of the unchanged moveset prefix. Another command targeting offset `0x000624D4` resolves to `0x087B68D4`. The precise *conditions* on each command mean values may be temporary or absent in idle states.

This explains why the unchanged small moveset header does not indicate the cheat is inactive; it does **not** prove any one pointer operation successfully executed in the user's session.

## Why anomalous behavior is still unresolved

- Speed-control word `0x08C52890=0x00000001` in both stock and mod snapshots.
- Camera speed `0x08D6F270`: stock `0x3DCCCCCD`, mod `0x3DD22600`.
- Zoom `0x08D6F224`: stock `0x42480000`, mod `0x42480002`.
- SlowMo state `0x08C07CA0`: stock `0x000007C8`, mod `0x000005BB`.
- Without synchronized gameplay/actual slowed-state observation, these differences are **not** sufficient to diagnose faulty slowdown.

## Next experiment

1. `trace-jin-runtime-links.mjs` probes eight selected conditional pointer-write destinations derived from the live moveset base; it **does not perform writes**.
2. Record `stock-idle` (cheats OFF) and `mod-idle` (master initialized, Jin runtime ON) at matching Jin idle state, both with ultrawide and expanded memory unchanged.
3. If no link matches in idle state, collect during the specific attack or animation showing unusual behavior using `--samples 20 --interval-ms 200` while manually triggering the move. A nonmatch while idle is expected for conditional codes.
4. Recheck camera with revised raw-code read mode, keep optional camera CWCheats off. Avoid repeated master activations just to chase an intermittent conditional pointer.
5. Record inputs/moves that look Tekken 8 inspired or visibly incorrect. No evidence yet establishes a working Rage Art.
