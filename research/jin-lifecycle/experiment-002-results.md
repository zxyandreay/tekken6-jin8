# Experiment 002 — stock and modded deep Jin memory captures

**Status: PARTIAL EVIDENCE ACCEPTED; A/B NOT CONTROLLED.** Captures supplied October 10, 2026, Tekken 6 `ULUS10466` version `1.00`, PPSSPP `v1.20.4`.

## Original artifacts (not committed)

| Snapshot | SHA-256 of JSON | Captured UTC |
| --- | --- | --- |
| `stock-jin-battle-deep.json` | `73659b04d1726908b07883471bbf8ff3e786ccce7ec6717fbd763c8666725e0d` | `2026-10-10T04:24:16.848Z` |
| `mod-jin-battle-deep.json` | `182f956145c8741d8accbba849a5a028e47ab0ebe5e2860e8d2b596d5c1a663b` | `2026-10-10T04:59:29.874Z` |

Local debugger endpoint/IP and raw diagnostic snapshots are intentionally excluded.

## Direct observation: master payload installed at two distant positions

- `0x09110000`: stock `00000000000000000000000000000000` → mod `9081b9089081b908e400110942080000`. Decodes to `0x08B98190`, `0x08B98190`, `0x091100E4`, `0x00000842`, exactly matching **master source lines 4–7**.
- `0x0A1F0AB0`: stock `memory.read: Invalid address` → mod `0e7cdf00000000000000000000000000`. The first word decodes to **`0x00DF7C0E`**, exactly matching **master source line 511094**, `_L 0x219F0AB0 0x00DF7C0E`.
- The source file has **511,607** master `_L` records. Two distant matches show payload construction has progressed into a late region; not that **every** record executed correctly.
- **CWCheat address conversion:** direct 32-bit opcode `0x2xxxxxxx` uses PSP guest user-memory base **`0x08800000`** plus the low 28-bit offset. Therefore `0x219F0AB0` maps to `0x0A1F0AB0`, *not* `0x099F0AB0`. Source max direct-write address is `0x0A1F0AB0`, beyond default 32 MiB RAM end `0x0A000000`. Expanded memory remains material.

## P1 Jin and pointer observations

- P1 character ID `0x09` in both.
- Candidate moveset pointer `0x08754400` in both; its **256-byte window is byte-identical**, SHA-256 `5b3803ae8bc868d53ba83804f88918d74cd070e1ac822a5667195fddf4525654`.
- Candidate current-move pointer `0x087AD474` in both. Its 128-byte window changes at just **five byte offsets**: `0x08`–`0x0B`, and `0x24`; the affected 32-bit words change `0x0962FEE0` → `0x0A000000` and `0x00000080` → `0x0000007A`.
- Candidate current move ID `0x8001`, incoming ID `0`, and input field `0x00FF0000` in both.
- **Both CPUs were running** (`paused:false`); moving animation-state fields can change legitimately. Do not interpret the two current-move record differences as a verified master hook.

## IMPORTANT confound: plugin and camera differ

- Stock snapshot modules: `tekken`, `scePsmf_library`, `scePsmfP_library` (**no ultrawide**).
- Mod snapshot modules: includes **`Tekken6Ultrawide`**.
- All four ultrawide aspect instruction pairs are stock (`0x3C013FE3/0x34218E39`) in stock snapshot; patched (`0x3C01400E/0x34214165`) in mod snapshot.
- Camera at `0x0895350C`: `0x3C013F82` in stock (consistent with optional `Camera - Wider`), `0x3C013F80` in mod (default preset; also used by Jin cheat).
- Mod-only snapshot includes candidate effect controller reads: speed `0x08C52890=0x00000001`, camera speed `0x08D6F270=0x3DCE0D2A`, zoom `0x08D6F224=0x42480000`, SlowMo timer `0x08C07CA0=0x00000470`. Stock capture predates `effectWords` probe addition, so there is **no paired stock effect baseline**. No conclusions about slowed state from these numbers alone.

## Follow-up

1. Cold-boot **stock Jin cheats OFF but ultrawide PRX ON** with memory `80` (same plugin, camera options OFF in both), and record a new *matched* deep stock sample. This removes the known plugin confound. No cheat activation in this control.
2. Use new `verify-master-payload.mjs` to sample 11 representative direct-write signatures through the full source master file in the same configuration: first with cheats OFF, later after known manual activation. A high matching fraction is evidence of distributed initialization, **not full verification**.
3. Capture `mod-jin-slowmo` if reliably triggered, and ideally record paused CPU state. The main first mod snapshot was not captured during a verified SlowMo event.
4. The user should report which gameplay inputs yielded modded moves, whether speed effects were intentional and temporary, and whether the unmodified game remains normal.

Next step after signatures: trace the **pointer installation and live runtime conditions**, not design a PRX hook from a static two-frame diff.
