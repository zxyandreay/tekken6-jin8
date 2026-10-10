# Experiment 001 — stock menu vs Jin battle

**Status: PARTIALLY ACCEPTED (stock control), mod A/B pending.**
Based on two user-provided read-only PPSSPP captures taken October 10, 2026. Raw captures are not committed because they contain a local debugger IP and machine-state diagnostics.

## Environment

- Tekken 6 USA `ULUS10466`, game version `1.00`.
- PPSSPP `v1.20.4`.
- `stock-menu`: `2026-10-10T04:12:45.956Z`; `stock-jin-battle`: `2026-10-10T04:14:22.286Z`.
- Both snapshots report `cpu.paused = false`; reads are sequential, not atomic.
- Module names in both: `tekken`, `scePsmf_library`, `scePsmfP_library`. No ultrawide module reported.

## Candidate fields

| Location and candidate meaning | Menu | Jin battle |
| --- | --- | --- |
| `0x08C078B4` P1 character ID | `0x00000000` | `0x00000009` |
| `0x08C078E0` moveset pointer | `0x08E40CD0` | `0x08754400` |
| `0x08C07964` current-move pointer | `0x00000000` | `0x087AD474` |
| `0x08C07A18` current-move ID | `0x00000000` | `0x00008001` |
| `0x08C07D1C` incoming-move ID | `0x00000000` | `0x00000000` |
| `0x08C07D24` incoming-move pointer | `0x00000000` | `0x00000000` |
| `0x08C07D3C` input/button field | `0x00000000` | `0x00FF0000` |
| `0x0895350C` shared camera word | `0x3C013F82` | `0x3C013F82` |

Five of the seven candidate character/move fields changed. The character ID `0x09` is consistent with Jin. The two nonzero pointers have *not* been dereferenced or structurally validated; they are promising probe targets, not confirmed hooks.

## Unchanged regions and bounds

- All eight ultrawide aspect-instruction words retain stock `0x3C013FE3` and `0x34218E39` values.
- 16 bytes at `0x09110000` are zero in both states (expected without the master mod).
- 16-byte samples at `0x08B98190` and `0x08E423B0` are identical in both states.
- Reading `0x0A1F0AB0` returns `memory.read: Invalid address` both times. Investigate PPSSPP expanded RAM setting and expected mod mapping; do not assert a specific memory requirement from this alone.

## Conclusions and next experiment

**Supported:** the stock game has usable remote debugger state, and active Jin correlates with `0x09` and two nonzero move-related pointers.

**Not supported yet:** verified table structure, correct allocation timing, exact mod injection hook, Rage Art state, or compatibility with a live Jin mod. The snapshots were not taken with the CPU paused.

**Next:** sample small, validated windows at the current moveset/current-move pointer targets in a stock fight and after successfully activating the original Jin master cheat. Record whether the mod actually changes Jin's available moves. Prefer manually paused PPSSPP during capture. The probes must not modify memory.
