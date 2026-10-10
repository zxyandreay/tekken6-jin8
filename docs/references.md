# External references and credits

**Status: SOURCE INDEX.** Read these as third-party background, not verified runtime evidence in our local build.

## Code and technical reference

- [PPSSPP developer tools](https://www.ppsspp.org/docs/development/developer-tools/) — enabling remote debugger.
- [PPSSPP WebSocket debug protocol source](https://github.com/hrydgard/ppsspp/blob/master/Core/Debugger/WebSocket.cpp) — subprotocol, JSON events and tickets.
- [PPSSPP memory event handlers](https://github.com/hrydgard/ppsspp/blob/master/Core/Debugger/WebSocket/MemorySubscriber.cpp) — `memory.read` and `memory.read_u32`.
- [Sadamitsu's Tekken research](https://www.sadamitsu.ru/) — moveset/move pointer and custom moves research.
- [Kiloutre/TKMovesets](https://github.com/Kiloutre/TKMovesets) — T6 structures and moveset extractor. Its public README documents Tekken 7 import support; **T6 PPSSPP import into T6 is not an established feature**.
- [TKMovesets releases](https://github.com/Kiloutre/TKMovesets/releases/tag/0.9.9.3) — T6 PPSSPP extraction support added June 24, 2023 (requires x64 PPSSPP and base address configuration).
- [Tekken Global Mod blog](https://pinoytekken.blogspot.com/2020/05/download-tekken-global-mod-season-2-dlc.html) — manual activation conventions; third-party instructions may be outdated.
- [Tekken 6 Ultrawide](https://github.com/zxyandreay/tekken6-ultrawide) — compatibility baseline and research-documentation model.

## Attribution notes

Community demonstrations credit **Sadamitsu** for modified animations and **Shokyaku** for cheat/code work on Jin-focused content. Credit alone is not a redistribution license. Obtain explicit permission where required before packaging third-party payloads or converted game animations.

## Licensing caution

Do not blindly vendor TKMovesets or PPSSPP code: inspect their current license terms before copying. Independent runtime observation and linking to documentation avoid most licensing uncertainty.
