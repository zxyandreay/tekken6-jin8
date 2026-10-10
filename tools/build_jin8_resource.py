#!/usr/bin/env python3
"""Generate standalone Jin8 PRX data files from an owned source CWCheat INI.
The 13MB third-party source is never committed to the public repository.
PSP data format: ASCII J8C1 + little-endian u32 line count + (u32,u32)*N.
"""
import argparse
import hashlib
import json
import re
import struct
from pathlib import Path

HEADER = struct.Struct("<4sI")
CODE = struct.Struct("<II")
PAIR = re.compile(r"^\s*_L\s+(0x[0-9a-fA-F]{1,8})\s+(0x[0-9a-fA-F]{1,8})", re.I)
MASTER = "(m)Tekken 8"
RUNTIME = "(1)jin"
EXPECTED_COUNTS = {MASTER: 511607, RUNTIME: 2458}
EXPECTED_SHA256 = "96541d37ad9ec8114c5f07feca63143d90cc0cdcda2218494cb19c970a8603d3"


def parse_source(path: Path):
    groups = {MASTER: [], RUNTIME: []}
    current = None
    for row in path.read_text(encoding="utf-8-sig", errors="replace").splitlines():
        if row.startswith("_C0 ") or row.startswith("_C1 "):
            name = row[4:].strip()
            current = name if name in groups else None
        elif row.lstrip().startswith("_L") and current is not None:
            match = PAIR.match(row)
            if not match:
                raise ValueError(f"Malformed _L record: {row[:100]}")
            groups[current].append(tuple(int(word, 16) for word in match.groups()))
    return groups


def build(source: Path, output: Path, allow_other_source=False):
    raw_hash = hashlib.sha256(source.read_bytes()).hexdigest()
    if raw_hash != EXPECTED_SHA256 and not allow_other_source:
        raise ValueError("Source fingerprint does not match the investigated INI. "
                         "Pass --allow-other-source only if you deliberately trust a changed source.")
    groups = parse_source(source)
    for name, count in EXPECTED_COUNTS.items():
        if len(groups[name]) != count:
            raise ValueError(f"{name}: expected {count} records, got {len(groups[name])}. "
                             "Do not deploy a partial master.")
    output.mkdir(parents=True, exist_ok=True)
    result = {"source_sha256": raw_hash, "format": "J8C1", "groups": {}}
    for name, filename in ((MASTER, "master.j8c"), (RUNTIME, "runtime.j8c")):
        target = output / filename
        with target.open("wb") as fp:
            fp.write(HEADER.pack(b"J8C1", len(groups[name])))
            for a, b in groups[name]:
                fp.write(CODE.pack(a, b))
        result["groups"][filename] = {
            "records": len(groups[name]),
            "sha256": hashlib.sha256(target.read_bytes()).hexdigest(),
            "bytes": target.stat().st_size,
        }
    (output / "resource-manifest.json").write_text(json.dumps(result, indent=2) + "\n", encoding="utf-8")
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path, help="Original ULUS10466 (3).ini file")
    parser.add_argument("--out", type=Path, default=Path("build/resource"))
    parser.add_argument("--allow-other-source", action="store_true")
    args = parser.parse_args()
    print(json.dumps(build(args.source, args.out, args.allow_other_source), indent=2))


if __name__ == "__main__":
    main()
