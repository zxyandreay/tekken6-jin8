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
