# Ultrawide compatibility findings

**Status: STATIC REVIEW ONLY — no live combined regression test in Jin8 yet.**

Reference repository: https://github.com/zxyandreay/tekken6-ultrawide

- v1.2.1 / OPT-EXP4A release baseline is constrained to a 0x0EB0 mapped PRX segment.
- The ultrawide plugin owns four aspect instruction pairs and several carefully classified HUD renderer pathways.
- The original Jin cheat and ultrawide camera presets may write guest `0x0895350C`; this needs explicit arbitration.
- Previous ultrawide research associated expanded plugin layout with replacement texture/font and fast-forward regressions.
- No direct master-write collision was observed against the four known aspect sites in the earlier analysis; this does **not** rule out indirect conflicts.
- No source or release files in the ultrawide repository should be changed by this investigation.

First actionable check: obtain stock, ultrawide-only, Jin-cheat-only, and combined captures at Jin's fight. Diagnose camera and moveset address changes **without patching any memory**.

## Confirmed code overlap — Investigation 002

The Jin runtime cheat writes to `0x1015350C` three times with `0x3F74`, `0x3F70`, and `0x3F80`, while optional ultrawide camera CWCheats write `0x3F80`–`0x3F83`. This is an explicit shared-camera conflict. Zoom/slowmo logic additionally exists inside the Jin runtime itself. See [Investigation 002](../cheat-format/investigation-002-effects.md). Do **not** infer the ultrawide PRX itself caused mixed moves without A/B testing.
