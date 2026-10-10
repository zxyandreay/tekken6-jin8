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
