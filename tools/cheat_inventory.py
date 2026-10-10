#!/usr/bin/env python3
"""Local-only CWCheat lexical inventory: reads the user's own cheat file; no writes to game or original."""
from __future__ import annotations
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import re

HEADER = re.compile(r"^_C([01])\s+(.+?)\s*$")
LINE = re.compile(r"^_L\s+0x([0-9A-Fa-f]{8})\s+0x([0-9A-Fa-f]{8})\s*$")


def summarize(file_path: Path) -> dict:
    contents = file_path.read_bytes()
    text = contents.decode("latin-1")
    groups: dict[str, dict] = {}
    current: str | None = None
    unparsed = 0
    for row in text.splitlines():
        row = row.strip()
        match = HEADER.fullmatch(row)
        if match:
            current = match.group(2)
            groups[current] = {"enabledAtStart": match.group(1) == "1", "records": 0, "families": Counter()}
            continue
        if row.startswith("_L"):
            parsed = LINE.fullmatch(row)
            if parsed is None:
                unparsed += 1
                continue
            if current is None:
                unparsed += 1
                continue
            opcode = int(parsed.group(1), 16)
            group = groups[current]
            group["records"] += 1
            group["families"][f"0x{opcode >> 28:X}"] += 1

    return {
        "sizeBytes": len(contents),
        "sha256": hashlib.sha256(contents).hexdigest(),
        "groups": groups,
        "totalRecords": sum(group["records"] for group in groups.values()),
        "unparsedRecords": unparsed,
        "warning": "Lexical opcode-family counts only; not an interpreter, execution trace, or payload decoder.",
    }


def main() -> None:
    parser = argparse.ArgumentParser(description="Inventory your local CWCheat file without uploading or modifying it.")
    parser.add_argument("file", type=Path, help="Source CWCheat INI path; never commit this file")
    args = parser.parse_args()
    result = summarize(args.file)
    print(json.dumps(result, indent=2))


if __name__ == "__main__":
    main()
