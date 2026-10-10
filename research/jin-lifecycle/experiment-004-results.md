# Experiment 004 — source conditional runtime links

**Status: RUNTIME POINTER WRITES OBSERVED; INPUT/ANIMATION CAUSALITY NOT YET PROVEN.**

User-reported read-only PPSSPP probe output on October 10, 2026; original source verified against local user-supplied `ULUS10466 (3).ini`. User supplied console text for `links-stock-idle`, `links-mod-idle`, and `links-mod-attack`; raw output JSON files have not yet been uploaded.

## Baseline differences

- Stock Jin `[0x08C078E0] = 0x08754400`.
- Modded Jin `[0x08C078E0] = 0x086A8800`.
- Difference: `0x000ABC00` (703,488 bytes), lower in mod capture.
- The existing probe reads and resolves the pointer **once at start** of each capture. Stock addresses start in `0x087...`; mod addresses are computed from `0x086A8800` and start in `0x086...`. Do not compare raw absolute destination addresses directly.
- This is an observed between-run difference, **not proof** that the master relocates the structure.

## Eight source-derived pointer-write offsets

| Source line | Relative offset | Expected | Stock idle | Mod idle | Mod attack (20 samples) |
| --- | --- | --- | --- | --- | --- |
| 511781 | `0x02C250` | `0x0909CE00` | `0x0875F554` | `0x086B3954` | 0/20 |
| 511791 | `0x02C270` | `0x0909CE10` | `0x0875F564` | **`0x0909CE10`** | **20/20** |
| 512751 | `0x05F9DC` | `0x090A5490` | `0` | `0` | 0/20 |
| 512830 | `0x03F974` | `0x08E423BC` | `0x0879C8C8` | **`0x08E423BC`** | **20/20** |
| 513124 | `0x066AF4` | `0x09096200` | `0x087A23B4` | **`0x09096200`** | **20/20** |
| 513567 | `0x069B94` | `0x0A02F5C4` | `0x09705100` | `0x09705100` | 0/20 |
| 513991 | `0x0624D4` | `0x0902E2B0` | `0x087A1340` | `0x086F5740` | 0/20 |
| 513997 | `0x0625DC` | `0x0902E2B0` | `0x087A1430` | `0x086F5830` | 0/20 |

There were **no memory-read errors** in these captures. Three exact matching custom values were observed after mod activation, stable in all 20 attack samples. **Stable observed values are not proof of 20 writes**; an earlier write might simply persist in memory.

## Source-code insight: both moveset base prefixes explicitly supported

The source repeatedly checks 16-bit high parts at guest `0x08C078E2`, including:

```text
_L 0xE001086A 0x004078E2
_L 0xE0010875 0x004078E2
```

In the master `(m)Tekken 8` group, **99** `0x086A` conditions and **99** `0x0875` conditions occur; the `(1)jin` runtime group has **43** each. The conditions occur in paired, equivalent-looking paths. This supports that both observed moveset prefixes are anticipated by the source mod. It does **not** prove which game factor determines the pointer allocation.

The older probe's first 256-byte moveset-header sampling was insufficient because these writes target offsets `0x02C250..0x069B94`.

## What the conditional gates imply

- Source line **512751**, expected `0x090A5490`, is gated by Jin P1 and current move ID **`0x0266`**, under a source comment `// CD`. A zero at idle or non-matching attack is expected.
- Source line **513567**, expected `0x0A02F5C4`, is gated by Jin P1 and current move ID **`0x043D`**, near a source comment `// 1+2+3+4`.
- Source lines **513991/513997** use current move ID **`0x02E3`**. No input mapping is proven by that source excerpt.
- Source line **513124** is in a broad Jin character conditional section and was present continuously; it is a useful persistent-control anchor.
- Source **511781** has several layered gates including `current move ID = 0x8001`; nonmatching values while idle cannot be treated as an error.
- The source includes camera and SlowMo modifications, but these probes do **not** capture a confirmed abnormal SlowMo event.

## Transient network problem

The mod-idle first attempt returned a WebSocket handshake timeout, followed by a successful retry. This is a **debugger transport issue**, not direct evidence of a moveset fault; do not confuse it with a cheat operation failure.

## Next experiment — record current move ID and runtime pointer targets together

A new read-only sampler `watch-jin-move-conditions.mjs` resolves the moveset base **on every sample**, records current/incoming move IDs, input state, speed/camera candidate state and selected source pointer-link values. This provides the minimum evidence to correlate a particular move transition with a conditional pointer write. The sample stores `movesetBaseAfter` to flag a mid-snapshot pointer change.

1. In a modded Jin fight, preferably Practice, start the sampler; then perform **one known repeatable move** and record the input. Use `--samples 40 --interval-ms 200` (bounded non-atomic reads).
2. If possible attempt the source-commented `1+2+3+4` action; check whether current move ID `0x043D` is observed. No guarantee this input produces that move under current state.
3. Repeat with a second source-mentioned move (e.g. `d/f+4`, source comments adjacent to the `0x0266` section) if successful or safe.
4. A matching pointer during a move transition is more informative than idle-only matching; a nonmatch can still be due to timing and condition skip.
5. Keep the ultrawide, expanded memory and camera-cheat configuration fixed and do not re-run the giant master repeatedly.
