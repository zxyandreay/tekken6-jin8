# Incident 001 — Android crash and font/texture regression in Jin8 v0.1

**Reported October 10, 2026.** Test device Xiaomi 23049PCD8G v35 (arm64), PPSSPP v1.20.4 release, Tekken 6 USA `ULUS10466` known-good CRC `ae9abfb0`.

## User-observed failure

- PPSSPP purple **Game crashed / Bad Execution Address**.
- `PC = 0xD9FF4A60`; `Destination = 0xD9FF4A60`; `RA = 0x08B4CFB8`.
- CPU core **JIT**; user-main thread, attr `0x800040FF`; Cheats=Y; Plugins=Y. The screenshot does **not** list which cheats were actually selected.
- With v0.1 Jin8 installed, user observes that previously working custom font/text replacement textures are no longer selected.
- No `Jin8.log`, module list, crash backtrace, PPSSPP memory map, exact startup phase, or shader/texture hash dump were supplied. **The root cause of the bad jump is not proven.**

## Prior validated ultrawide lessons (not mistaken for proof of Jin8 mechanism)

- `tekken6-ultrawide` research **OPT-EXP2R2** rejected a hard-coded `jal 0x08800640` to PRX-local code that failed due to module relocation. Jin8 source does *not* contain this same fixed `jal` approach, so that historical crash cannot be directly transposed.
- `research/autohud/archive/test-builds.md` documents a *controlled resident-size-only* experiment changing **`p_memsz 0x0EB0 → 0x12D0`** while keeping accepted ultrawide code and instructions unchanged; font/texture replacement selection regressed on device. This was a controlled result, not just a working theory. The accepted ultrawide v1.2.1 has one `PT_LOAD`, `p_filesz=p_memsz=0x0EB0`, SHA-256 `6a537691e758dabc912910b9cfb1758e9ea9b5a2554242a43ff87e281c32a8e8`.
- Earlier oversized ultrawide tests also changed fast-forward behavior.
- Therefore module footprint **must be treated as an actual compatibility risk**. No edits are authorized to the accepted ultrawide binary or its source/release.

## Confirmed implementation risks in Jin8 v0.1

- PRX `Tekken6Jin8.prx` file size **10,454 B**, one `PT_LOAD`, `p_filesz=0x01D30`, **`p_memsz=0x06A08`** (27,144 B). `.bss` includes a **2,458 × 8 = 19,664 B** always-resident runtime-code array, on top of local I/O reader buffers, stack, and PRX code.
- In the v0.1 `execute()` the loader called `sceKernelDcacheWritebackInvalidateAll()` every cycle and `sceKernelIcacheInvalidateAll()` whenever direct game-code writes occurred, which may happen repeatedly in the 50-ms runtime interpreter loop. This causes broad repeated invalidation instead of targeted changed-site updates.
- Direct writes to guest addresses `0x08C...` and pointer targets might destabilize game memory if executed in a different lifecycle phase than the original CWCheat engine. No PPSSPP crash stack has been captured to establish which operation.
- The older `CWCheat` manual mod was already reported to trigger abnormal camera/slow-motion and some missing custom font textures, so **source mod behavior and PRX mapping may both contribute**; do not attribute everything to module size without isolation.
- The invalid `0xD9FF4A60` PC is outside supported guest execution mapping. Its exact origin (corrupted indirect pointer, bad jump/relocation, game state, JIT, plugin overlap, etc.) is unresolved.

## v0.2 experimental implementation response

- Remove the permanently resident 19,664-byte Jin runtime table. Stream runtime records from the plugin's `runtime.j8c` resource in **128-record buffers**, reopening for each tick; source and instruction order preserved.
- Use changed-value checks and address-specific D/I cache invalidation **only for modified code sites in `0x08800000..0x08A00000`**; remove global D/I-cache invalidation on every execution pass. Code/data cache semantics still require device validation.
- Reduce dedicated loader thread stack from `0x8000` to `0x4000` and adjust priority. Leave official ultrawide release unmodified.
- Require initialized current move ID before master attempt; abort on unreadable or truncated resources; avoid endlessly reapplying master on failure.
- Provide optional **no-write `Jin8.safe` mode**, while retaining the exact same PRX and manifest. Creating the empty marker file prevents *all* mod-data writes. With the marker, if fonts still fail, suspect PRX footprint/memory setting and texture hash mapping rather than Jin runtime writes; if fonts recover, suspect runtime injection. This tests only one aspect and is not a working mod mode.
- Compiled PRX v0.2: file size **10,902 B**, single `PT_LOAD`, `p_filesz=0x01DD0`, **`p_memsz=0x01DD8`**, a decrease of `0x4C30` / ~72%. **Still larger than ultrawide's tested safe `0x0EB0` footprint; therefore texture compatibility is not proven.**
- No new cheat data or executable modifications made to `tekken6-ultrawide`.

## Test and rollback

1. Back up saves and PPSSPP Memory Stick, keep original ultrawide v1.2.1 installed **without editing its manifest or PRX**.
2. Replace only `PSP/PLUGINS/Tekken6Jin8/` with the v0.2 package. Disable both original Jin CWCheat groups, leave camera presets OFF, cold boot.
3. If crash or custom fonts fail, do **not** keep rerunning the unguarded master. Create an empty `Jin8.safe` in Jin8 folder and cold boot. Check game start/menus, font textures, ultrawide HUD; record `Jin8.log`.
4. If safe mode remains problematic, disable only Jin8 plugin (not ultrawide), cold boot and confirm that old fonts/textures return; this is a plugin-footprint/memory-layout A/B. Do not overwrite/modify texture mappings as a workaround.
5. Ask for `Jin8.log`, crash timing, whether both Jin CWCheats were OFF, and if safe mode restores normal operation. Proceed from evidence, not claimed resolution.

**v0.2 passes PSPSDK compilation only; no actual Android runtime regression test has yet been performed.**

## Future engineering options if v0.2 still fails

- Reduce Jin8's mapped ELF segment even further with a small bootstrap plus deferred/relocated loader, ensuring no conflict with code/data payload ranges; measure `PT_LOAD` and avoid adding allocations into resource-sensitive game memory before the texture/font init.
- Narrow source runtime writes to a verified phase and/or enforce whitelist of ranges/expected words, then reconcile differences against PPSSPP's actual CWCheat interpreter.
- Capture precise local crash timing, PC/RA backtrace and plugin load addresses before declaring fixed.
