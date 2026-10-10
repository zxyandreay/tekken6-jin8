# Documentation

**Status: ACTIVE RESEARCH** — Canonical reference material for the Jin8 project.

## Reading order

1. [Roadmap](roadmap.md): experiments, milestones, exit criteria.
2. [Memory map](architecture/memory-map.md): verified identifiers, candidate addresses, and hazards.
3. [Ultrawide compatibility](architecture/ultrawide-compatibility.md): non-interference contract for the separate plugin.
4. [Remote debugging](development/remote-debugging.md): repeatable, read-only capture workflow.
5. [Research methodology](development/research-methodology.md): evidence and acceptance rules.
6. [Sources](references.md): external technical references and credits.

## Policy

- `docs/` describes the **current model and intended contract**.
- `research/` contains detailed findings, experiments, and explicitly unresolved hypotheses.
- Record failed tests. Do not transform an assumption into a fact without independent device evidence.
- The existing `tekken6-ultrawide` repository is **read-only context**, never an implementation dependency.
