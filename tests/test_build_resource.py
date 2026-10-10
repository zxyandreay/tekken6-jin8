"""Offline parser tests. No copyrighted/third-party cheat data is committed."""
import tempfile
import unittest
from pathlib import Path
from tools.build_jin8_resource import parse_source, build, MASTER, RUNTIME

class TestResourceBuilder(unittest.TestCase):
    def test_two_named_sections_comments_and_other_sections(self):
        source = (
            "_S ULUS-10466\n"
            "_C0 ignored\n_L 0x20000000 0x12345678\n"
            "_C0 (m)Tekken 8\n"
            "_L 0xE0400009 0x004078B4\n"
            "_L 0x20910000 0x08B98190 // comment\n"
            "_C1 (1)jin\n_L 0xE01D08C0 0x204078E2\n"
        )
        with tempfile.TemporaryDirectory() as folder:
            file = Path(folder) / "source.ini"
            file.write_text(source)
            groups = parse_source(file)
            self.assertEqual(groups[MASTER], [(0xE0400009, 0x004078B4), (0x20910000, 0x08B98190)])
            self.assertEqual(groups[RUNTIME], [(0xE01D08C0, 0x204078E2)])
            with self.assertRaisesRegex(ValueError, "expected 511607"):
                build(file, Path(folder) / "out", allow_other_source=True)

    def test_rejects_unverified_input(self):
        with tempfile.TemporaryDirectory() as folder:
            file = Path(folder) / "source.ini"
            file.write_text("_C0 (m)Tekken 8\n")
            with self.assertRaisesRegex(ValueError, "fingerprint"):
                build(file, Path(folder) / "out")

if __name__ == "__main__":
    unittest.main()
