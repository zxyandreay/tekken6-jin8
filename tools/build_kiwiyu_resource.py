#!/usr/bin/env python3
"""Convert the user's original KiwiYu Jin 1.0 ZIP to J8C1 streams.

Run: python tools/build_kiwiyu_resource.py "Jin 1.0 by KiwiYu.zip" --out kiwi-build
No original third-party mod bytes are committed to this repo.
"""
import argparse
import hashlib
import json
import re
import struct
import zipfile
from pathlib import Path

ORIGINAL_INI_SHA256 = "5a1e6eb63e50341d35bc5d8d283d499d0491420eb3bd7f66e183b596320b6193"
EXPECTED = {
    "JIN MASTER CODES": ("jin_master.j8c", 436692),
    "TEKKEN 8 MASTER CODES": ("tekken8_master.j8c", 437310),
    "JIN Moves": ("jin_moves.j8c", 5113),
}
LINE = re.compile(r"_L\\s+0x([0-9a-fA-F]{1,8})\\s+0x([0-9a-fA-F]{1,8})(?=\\s|_|$)")


def convert(archive: Path, out: Path) -> dict:
    with zipfile.ZipFile(archive) as z:
        source = z.read("ULUS10466.ini")
    actual = hashlib.sha256(source).hexdigest()
    if actual != ORIGINAL_INI_SHA256:
        raise ValueError("Source checksum does not match the verified KiwiYu upload; refusing to convert")
    parsed = {k: [] for k in EXPECTED}
    current = None
    recovered = []
    for number, line in enumerate(source.decode("utf-8-sig").splitlines(), 1):
        line = line.strip()
        if line.startswith("_C"):
            current = line[4:].strip()
        elif "_L" in line:
            pairs = LINE.findall(line)
            if not pairs:
                raise ValueError(f"Malformed cheat line {number}: {line[:80]}")
            if len(pairs) > 1:
                recovered.append(number)
            if current in parsed:
                parsed[current].extend((int(a, 16), int(b, 16)) for a, b in pairs)
    if recovered != [799754]:
        raise ValueError(f"Unexpected joined-line structure: {recovered}")
    out.mkdir(parents=True, exist_ok=True)
    result = {"ini_sha256": actual, "joined_source_line_recovered": recovered, "groups": {}}
    for group, (filename, expected_count) in EXPECTED.items():
        records = parsed[group]
        if len(records) != expected_count:
            raise ValueError(f"{group}: expected {expected_count}, got {len(records)}")
        target = out / filename
        with target.open("wb") as f:
            f.write(struct.pack("<4sI", b"J8C1", len(records)))
            for a, b in records:
                f.write(struct.pack("<II", a, b))
        result["groups"][filename] = {
            "source_group": group, "records": len(records),
            "bytes": target.stat().st_size,
            "sha256": hashlib.sha256(target.read_bytes()).hexdigest(),
        }
    (out / "PAYLOAD-MANIFEST.json").write_text(json.dumps(result, indent=2) + "\\n")
    return result


def main():
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("archive", type=Path)
    p.add_argument("--out", type=Path, default=Path("kiwi-build"))
    args = p.parse_args()
    print(json.dumps(convert(args.archive, args.out), indent=2))


if __name__ == "__main__":
    main()
