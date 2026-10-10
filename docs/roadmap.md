# Roadmap

**Status: ACTIVE RESEARCH — not yet implemented**

## Objective

Make Tekken 8-inspired Jin moves and Rage Art load automatically for Tekken 6 USA (ULUS10466) under PPSSPP, with **no manual master-cheat toggling** and no regressions in the separately installed ultrawide plugin.

## Architecture hypothesis

A small, independent `Tekken6Jin8.prx` will:
1. Verify supported game build and lifecycle state.
2. Wait for Jin's real moveset structures to be allocated.
3. Stage a versioned external data pack into validated memory.
4. Establish or re-establish custom move tables/pointers only when proven safe.
5. Implement necessary persistent behavior now performed by the Jin CWCheat.
6. Detect transitions, prevent duplicate application, and restore only changes it owns.

This is a **design target**, not a demonstrated plugin. A short-term cheat interpreter may be used to establish behavioral parity first; a native table-aware loader is a subsequent optimization.

## Milestones and gates

| Phase | Work | Evidence required |
| --- | --- | --- |
| 0. Baseline | Clean Jin, ultrawide-only, original Jin cheat, combined baseline | Snapshot matrix, PPSSPP version/config, video or move observations |
| 1. Cheat analysis | Decode conditional/write/pointer operations, master regions, persistent writes | Deterministic parser tests, range manifest, no unbounded writes |
| 2. Lifecycle | Identify character/moveset initialization and memory resets | Repeated read-only snapshots and narrow debugger trace |
| 3. Tiny proof | Auto-activate **one verified move** | Repeated cold boot / next-match passes with identical behavior |
| 4. Loader | External packed data + runtime operations | Memory equivalence checks vs source mod |
| 5. Rage Art | Resolve camera, animation, reaction, state machine | Full successful and unsuccessful activation paths |
| 6. Release | Compatibility, save state, performance, recovery tests | Full regression matrix and installation steps |

## Early success criteria

- Stock `ULUS10466` is recognized and wrong games are refused.
- With other characters selected, no custom Jin data is applied.
- A test move operates without opening or toggling Cheats in the current match.
- Reload/next match does not retain stale pointers or crash.
- Independent `tekken6-ultrawide` v1.2.1 works, including replacement fonts/textures and fast-forward.
- A Rage Art test returns camera/game state to normal after successful and interrupted sequences.

## Risk register

| Risk | Mitigation |
| --- | --- |
| Master has ~511K records | Convert into compact staged blocks; do not continuously replay |
| Runtime conditional semantics unclear | Capture and test each opcode group before porting |
| Memory size/ownership | Detect validity; verify required expanded memory; bound ranges and pointer dereferences |
| Shared camera setting | Define ownership and restore policy before writing |
| PRX footprint affects replacement textures | Keep ultrawide PRX unchanged; audit loader allocation/mapping |
| JIT instruction replacement hides stock values | Use `memory.read` with `replacements:false` for code inspection |
| Source asset redistribution | Keep source payload, ISO, EBOOT, and extracted assets out of repository |

## Deliberately out of scope (first version)

Other regional game builds, gameplay networking guarantees, exact Tekken 8 engine behavior, and full support for arbitrary character mods.
