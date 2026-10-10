# Research

The experimental evidence trail for the Jin8 project.

- [Cheat source inventory](cheat-format/findings.md) — static analysis, opcode groups, open questions.
- [EBOOT fingerprint](eboot/fingerprint.md) — identity of the user-supplied decrypted EBOOT; never distribute the binary.
- [Jin lifecycle experiment](jin-lifecycle/README.md) — first remote debugger capture matrix.
- [Ultrawide integration notes](ultrawide/findings.md) — shared resources and known regression risks.

When a hypothesis is demonstrated, update the canonical file in `docs/` and preserve its proof here. Archive rejected experiments rather than erasing them.

- [Experiment 001: stock captures](jin-lifecycle/experiment-001-results.md) — confirmed baseline observations, not a mod test.

- [Experiment 002: deep stock vs mod results](jin-lifecycle/experiment-002-results.md) — first and late master signatures match; plugin confound pending.

- [Experiment 003: matched payload + pointer-runtime hypothesis](jin-lifecycle/experiment-003-results.md) — 11/11 mod signatures and next read-only probe.

- [Experiment 004: conditional runtime move links](jin-lifecycle/experiment-004-results.md) — three custom links observed, source supports two moveset base prefixes.

- [Experiment 005: cinematic-state event](jin-lifecycle/experiment-005-results.md) — speed mode 3 and zoom 16.0 match original SlowMo cheat; P2 interference documented.

- [Incident 002: v0.2 Jin fight crash; v0.3 native-cheat lightweight gate](jin-lifecycle/incident-002-v02-crash-lite-v03.md) — custom font regression and new implementation test.

- [Experiment 006: v0.4 fully standalone no-Cheats gameplay optimization](jin-lifecycle/experiment-006-v04-standalone-optimization.md) — source, benchmarks/footprints, caveats and test plan.

- [Incident 003: v0.4 prematch PC=0x10 crash and guarded v0.4.1](jin-lifecycle/incident-003-v04-prematch-null-jump.md).

- [Incident 004: v0.4.1 master-resume crash and v0.4.2 conflict guard](jin-lifecycle/incident-004-v041-master-resume-crash.md).

- [Incident 005: v0.4.2 first-runtime complete, partial JIT invalidation fixed in v0.4.3](jin-lifecycle/incident-005-v042-jit-coverage.md).

- [Incident 006: v0.4.3 still PC=0x10; master-only control is next](jin-lifecycle/incident-006-v043-repeat-null-jump.md).
