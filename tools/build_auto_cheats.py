#!/usr/bin/env python3
"""Create a one-time gated master and always-on-but-guarded Jin runtime CWCheat.
The tiny companion PRX sets only a sentinel; PPSSPP itself executes all cheat
records on its normal scheduler. Source bytes never enter the Git repository.
"""
from __future__ import annotations

import argparse
import hashlib
import re
from pathlib import Path

EXPECTED_SHA = "96541d37ad9ec8114c5f07feca63143d90cc0cdcda2218494cb19c970a8603d3"
MASTER_LINES = 511607
RUNTIME_LINES = 2458
# PSP guest 0x0A1FFFC0 minus CWCheat base 0x08800000.
GATE_OFFSET = 0x019FFFC0
GATE_CHECK = f"0x{GATE_OFFSET:08X}"
LAST_MASTER_OFFSET = 0x019F0AB0
GROUP = re.compile(r"^_C[01]\s+(.*)")
PAIR = re.compile(r"^\s*_L\s+(0x[0-9a-fA-F]{1,8})\s+(0x[0-9a-fA-F]{1,8})", re.I)
CAMERA = (
    "_C0 Camera - Original (Default)\n_L 0x1015350C 0x00003F80\n\n"
    "_C0 Camera - Slightly Wider\n_L 0x1015350C 0x00003F81\n\n"
    "_C0 Camera - Wider\n_L 0x1015350C 0x00003F82\n\n"
    "_C0 Camera - Widest\n_L 0x1015350C 0x00003F83\n"
)


def original_groups(source: Path, allow_modified=False):
    digest = hashlib.sha256(source.read_bytes()).hexdigest()
    if digest != EXPECTED_SHA and not allow_modified:
        raise ValueError("Unexpected cheat source SHA256; refusing to convert an unverified version.")
    names = {"(m)Tekken 8": [], "(1)jin": []}
    current = None
    for row in source.read_text(encoding="utf-8-sig", errors="replace").splitlines():
        match = GROUP.match(row)
        if match:
            current = match.group(1).strip()
        elif row.lstrip().startswith("_L") and current in names:
            pair = PAIR.match(row)
            if not pair:
                raise ValueError(f"Invalid source code line: {row[:100]}")
            names[current].append(tuple(int(x, 16) for x in pair.groups()))
    master, runtime = names["(m)Tekken 8"], names["(1)jin"]
    if len(master) != MASTER_LINES or len(runtime) != RUNTIME_LINES:
        raise ValueError(f"Invalid record count master={len(master)}, runtime={len(runtime)}")
    return master, runtime, digest


def safe_boundaries(lines):
    # Inserting a new condition inside an original E0/E1 skip window would
    # alter which physical records the original condition skips.
    n = len(lines)
    delta = [0] * (n + 2)
    for i, (op, _) in enumerate(lines):
        if op >> 28 == 0xE:
            maximum = 0xFF if op >> 24 == 0xE1 else 0xFFF
            skip = (op >> 16) & maximum
            first, last = i + 1, min(n, i + skip)
            if first <= last:
                delta[first] += 1
                delta[last + 1] -= 1
    count = 0
    safe = [False] * (n + 1)
    for boundary in range(1, n):
        count += delta[boundary]
        safe[boundary] = count == 0
    safe[n] = True
    return safe


def split_master(lines, max_chunk=3800):
    if max_chunk > 4095 or max_chunk < 128:
        raise ValueError("CWCheat E0 skip count is at most 4095")
    safe = safe_boundaries(lines)
    sections = []
    start = 0
    while start < len(lines):
        end = min(len(lines), start + max_chunk)
        while not safe[end] and end > start:
            end -= 1
        if end <= start:
            raise ValueError(f"No safe gated chunk end within {max_chunk} lines after {start}")
        sections.append((start, end))
        start = end
    return sections


def line(pair):
    return f"_L 0x{pair[0]:08X} 0x{pair[1]:08X}\n"


def generate(source: Path, target: Path, allow_modified=False):
    master, runtime, digest = original_groups(source, allow_modified)
    chunks = split_master(master)
    target.parent.mkdir(parents=True, exist_ok=True)
    with target.open("w", encoding="ascii", newline="\n") as output:
        output.write("_S ULUS-10466\n_G Tekken 6 [USA]\n\n")
        # Preserve the accepted ultrawide release's optional camera choices.
        output.write(CAMERA + "\n")
        output.write("_C1 Jin8 Auto Master (One-Time)\n")
        for start, end in chunks:
            count = end - start
            output.write(f"_L 0xE{count:03X}0001 {GATE_CHECK}\n")
            for item in master[start:end]:
                output.write(line(item))
        # Finish the master only if both flag=1 and late payload is present.
        output.write(f"_L 0xE0020001 {GATE_CHECK}\n")
        output.write(f"_L 0xE0017C0E 0x{LAST_MASTER_OFFSET:08X}\n")
        output.write(f"_L 0x{(0x20000000 | GATE_OFFSET):08X} 0x00000002\n\n")
        output.write("_C1 Jin8 Auto Runtime (After Master)\n")
        output.write(f"_L 0xE{len(runtime):03X}0002 {GATE_CHECK}\n")
        for item in runtime:
            output.write(line(item))
    return {"source_sha256": digest, "master_records": len(master),
            "runtime_records": len(runtime), "master_chunks": len(chunks),
            "output_sha256": hashlib.sha256(target.read_bytes()).hexdigest(),
            "bytes": target.stat().st_size}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("source", type=Path)
    parser.add_argument("--out", type=Path, default=Path("build/auto/ULUS10466.ini"))
    parser.add_argument("--allow-modified", action="store_true")
    args = parser.parse_args()
    print(generate(args.source, args.out, args.allow_modified))


if __name__ == "__main__":
    main()
