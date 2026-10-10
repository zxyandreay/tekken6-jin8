# Original Jin source cheat: static inventory

**Status: STATIC FACTS VERIFIED; RUNTIME SEMANTICS NOT YET PROVEN.**

Input supplied during prior project work (local, not committed): `ULUS10466 (3).ini`.
SHA-256: `96541d37ad9ec8114c5f07feca63143d90cc0cdcda2218494cb19c970a8603d3`.
File size: **13,372,511 bytes**.

## Sections and instruction volume

| Section | CWCheat `_L` lines | Initial declaration |
| --- | ---: | --- |
| `(m)Tekken 8` | **511,607** | `_C0` (not enabled by default) |
| `(1)jin` | **2,458** | `_C1` (enabled by default) |
| **Total** | **514,065** | — |

Opcode-family tally, based **only on the high nibble of the first CWCheat word** across both groups:

| High nibble | Count |
| --- | ---: |
| `0x2` | 512,129 |
| `0xE` | 1,256 |
| `0x0` | 448 |
| `0x6` | 194 |
| `0x5` | 15 |
| `0x3` | 11 |
| `0x1` | 8 |
| `0x4` | 4 |

These are **lexical instruction records**, not evidence that every line executes. Conditional lines and multi-line CWCheat opcodes require full interpretation.

The master starts:

```text
_C0 (m)Tekken 8
_L 0xE0400009 0x004078B4
_L 0x20910000 0x08B98190
_L 0x20910004 0x08B98190
_L 0x20910008 0x091100E4
```

The Jin runtime group begins:

```text
_C1 (1)jin
_L 0xE01D08C0 0x204078E2
_L 0xE01D0009 0x004078B4
_L 0xE01D039B 0x00408348
```

**Interpretation**: Master is a large data-deployment candidate; Jin group is a persistent control candidate. These roles are **hypotheses** until controlled memory diffs validate them. High-address write targets suggest expanded PSP memory may be required; exact requirement is unproven.

## Questions to resolve

1. Which writes are one-time payload construction, and which install game-visible pointers?
2. Are all master pages needed for a given Jin move? Which opcodes are conditional or sequence carriers?
3. What exactly does enabling/disabling the master do to game state?
4. Which Jin runtime writes must remain active every emulated frame?
5. Which instructions touch camera, VFX, load transitions, or other character data?
6. Are there idempotent reinitialization points on subsequent rounds, matches, and savestate loads?

## Local source audit

Do **not** commit the source cheat. You can run `python tools/cheat_inventory.py "C:\\path\\to\\ULUS10466.ini"` on your own licensed/user-supplied source file to regenerate counts and its hash. The tool only outputs descriptive metadata.

## Follow-up: user-observed zoom/slow motion

The runtime `(1)jin` block has explicit camera and slow-motion sections, and three writes to `0x1015350C`. See [Investigation 002](investigation-002-effects.md). The source bundle is not a verified complete mod distribution; do not assert all Tekken 8 moves are installed.
