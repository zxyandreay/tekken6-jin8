# Decrypted EBOOT baseline

**Status: STATIC VERIFICATION.** User-supplied file analyzed locally; deliberately not stored in GitHub.

- Name: `ULUS10466_EBOOT(6).BIN`
- Size: **4,158,480 bytes**
- SHA-256: `e0ed418952c51032796ebc54584592a9f6075495229f6d12d5065f3186bcb55f`
- Format: ELF32, little-endian MIPS
- Entry point: `0x08804124`
- PSP executable load address: `0x08804018`
- Program load segments: 1

The file contains the stock words at the four aspect-correction pairs documented in the separate ultrawide project:

```text
LUI 0x3C013FE3
ORI 0x34218E39
```

At: `0x08945F10/14`, `0x08946794/98`, `0x08946BC8/CC`, `0x08947D90/94`.

These are **static compatibility landmarks**, not proof of identical runtime state or source-cheat success. Never upload the decrypted EBOOT or game ISO to the repository.
