# Memory map: ULUS10466

**Status: MIXED — static verified sites and runtime hypotheses.**
All addresses below are **PSP guest virtual addresses**; they are not Windows host pointers. Values are specific to this game build and can change across lifecycle events.

## Candidate Jin/runtime data locations

These addresses are published in Tekken 6 modification references or appear as operands in the supplied source cheat. Their **meaning in a particular running state must still be verified**.

| Guest address | Tentative meaning | Evidence/confidence |
| --- | --- | --- |
| `0x08C078B4` | P1 character ID; Jin `0x09` | Candidate from source-cheat conditional and public research |
| `0x08C078E0` | P1 moveset-related pointer | Candidate from public research; dereference **not** yet validated |
| `0x08C07964` | Current move pointer | Candidate only |
| `0x08C07A18` | Current move identifier | Candidate only |
| `0x08C07D1C` | Incoming move identifier | Candidate only |
| `0x08C07D24` | Incoming move pointer | Candidate only |
| `0x08C07D3C` | Input/button state | Candidate only |

The first probe records 32-bit values at these sites but **does not follow any pointer**. Read errors (including invalid memory while at character select) are useful diagnostic data, not grounds to force access.

## Ultrawide-owned sites

Known instruction pairs in stock EBOOT, all observed as `LUI 0x3C013FE3` then `ORI 0x34218E39` in the supplied executable:

- `0x08945F10 / 0x08945F14`
- `0x08946794 / 0x08946798`
- `0x08946BC8 / 0x08946BCC`
- `0x08947D90 / 0x08947D94`

**Read-only diagnostics allowed. Patching not allowed** from Jin8. The original `tekken6-ultrawide` code and released PRX own these sites.

## Camera and source-cheat payload samples

- `0x0895350C`: camera preset location in ultrawide's optional CWCheats and writes within the source Jin code. **Known collision risk**, not resolved.
- `0x09110000`: first master payload target from the supplied `(m)Tekken 8` group.
- `0x08B98190`, `0x08E423B0`: observed operands/references in initial master data. **Not necessarily writable staging regions.**
- `0x0A1F0AB0`: high-address write target found in the supplied cheat (expand-memory implications to verify).

## Important distinctions

1. CWCheat addresses such as `0x204078E0` encode an operation plus an address offset; do not treat the entire CWCheat word as a PSP virtual address.
2. A plausible pointer value is not evidence that it points to the correct struct.
3. Runtime `memory.read_u32` may show PPSSPP replacement/JIT details at code sites; use `memory.read` with `replacements:false` to inspect unmodified guest instruction bytes.
4. Candidate character and current-move fields should be sampled at **menu, select, fight, round transition, and after master activation** before making a lifecycle hook.

## Baseline caveat — camera preset and high memory

The October 10 stock-labeled captures show camera word `0x3C013F82`, matching the low halfword of ultrawide's optional **Camera - Wider** preset (`0x3F82`). Consequently, do not call these captures fully camera-cheat-free. The high target `0x0A1F0AB0` is roughly 33.94 MiB above `0x08000000`, beyond 32 MiB, and was invalid in both observed states. Confirm expanded-memory setting during mod tests.

## Verified CWCheat address mapping and late master record

The source's 32-bit direct-write opcode `_L 0x219F0AB0 0x00DF7C0E` corresponds to guest **`0x0A1F0AB0`**, because CWCheat address offsets are based on `0x08800000` (not `0x08000000`). This was confirmed in the user's October 10 mod snapshot as little-endian `0e7cdf00`, and matches the highest direct target discovered in the 511,607-record master. The default RAM window ends before this address; expanded-memory dependency is a real issue for complete payload loading. See [Experiment 002](../../research/jin-lifecycle/experiment-002-results.md).

The first 256 bytes at `0x08754400` were unchanged between stock and mod snapshots; that alone is **not** proof the active move table is unmodified.
