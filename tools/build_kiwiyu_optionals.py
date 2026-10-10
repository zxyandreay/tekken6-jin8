#!/usr/bin/env python3
"""Create optional Kiwi Yu cheats + exact ultrawide camera presets from owned inputs.
Does not include the core Kiwi Yu moveset: that is handled by the standalone PRX.
Nothing in this script downloads or republishes original cheat archives.
"""
import argparse
import hashlib
import re
import zipfile
from pathlib import Path

SOURCE_HASH = "5a1e6eb63e50341d35bc5d8d283d499d0491420eb3bd7f66e183b596320b6193"
KIWI_OPTIONAL = (
    "No MUSIC",
    "P1 Rage Turns On",
    "Turn off Hazards",
    "Heat Meter (Alboss's Textures Required)",
)
CAMERAS = {
    "Camera - Original (Default)": "0x00003F80",
    "Camera - Slightly Wider": "0x00003F81",
    "Camera - Wider": "0x00003F82",
    "Camera - Widest": "0x00003F83",
}


def groups_from(text):
    groups = {}
    key = None
    for line in text.splitlines():
        if line.startswith(("_C0 ", "_C1 ")):
            key = line[4:].strip()
            if key in groups:
                raise ValueError("Duplicate source group: " + key)
            groups[key] = [line]
        elif key is not None:
            groups[key].append(line)
    return groups


def make_optional(source_zip: Path, ultrawide_ini: Path, destination: Path):
    with zipfile.ZipFile(source_zip) as archive:
        original = archive.read("ULUS10466.ini")
    if hashlib.sha256(original).hexdigest() != SOURCE_HASH:
        raise ValueError("KiwiYu archive INI fingerprint mismatch")
    kiwi = groups_from(original.decode("utf-8-sig"))
    ultra = groups_from(ultrawide_ini.read_text(encoding="utf-8-sig"))
    selected = []
    for name in KIWI_OPTIONAL:
        lines = kiwi.get(name)
        if not lines or not lines[0].startswith("_C0 "):
            raise ValueError("Missing or auto-enabled KiwiYu optional group: " + name)
        selected.append("\n".join(lines).rstrip())
    for name, value in CAMERAS.items():
        lines = ultra.get(name)
        if not lines or lines[0] != "_C0 " + name:
            raise ValueError("Missing camera entry: " + name)
        codes = [l.strip().upper() for l in lines[1:] if l.strip().startswith("_L ")]
        if codes != ["_L 0X1015350C " + value.replace("0x", "0X")]:
            raise ValueError("Ultrawide camera preset has changed: " + name)
        selected.append("\n".join(lines).rstrip())
    result = ("_S ULUS-10466\n_G Tekken 6 [USA]\n\n"
              "; Optional extras only. Core Jin8 handled by standalone KiwiYu PRX.\n"
              "; All groups are _C0 disabled by default.\n\n"
              + "\n\n".join(selected).rstrip() + "\n")
    if len(re.findall(r"^_C0 ", result, re.M)) != 8:
        raise ValueError("Unexpected number of groups")
    if re.search(r"^_C1 ", result, re.M):
        raise ValueError("Optional file must not auto-enable cheats")
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_text(result, encoding="utf-8")
    return destination


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("kiwiyu_zip", type=Path)
    parser.add_argument("ultrawide_ini", type=Path)
    parser.add_argument("--out", type=Path, default=Path("OPTIONAL-CHEATS/ULUS10466.ini"))
    args = parser.parse_args()
    target = make_optional(args.kiwiyu_zip, args.ultrawide_ini, args.out)
    print(target)


if __name__ == "__main__":
    main()
