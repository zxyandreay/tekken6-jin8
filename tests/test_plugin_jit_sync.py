"""Minimal static regression test for the v0.4.3 JIT coherency fix.
Not a simulator or proof of Android compatibility.
"""
import unittest
from pathlib import Path

SOURCE = Path(__file__).resolve().parents[1] / "plugin-standalone" / "src" / "main.c"

class TestJitSync(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.src = SOURCE.read_text(encoding="utf-8")

    def test_all_changed_direct_writes_are_tracked(self):
        self.assertIn("mark_dirty(a, (uint32_t)size);", self.src)
        self.assertNotIn("if (a >= 0x08800000u && a < 0x08A00000u)", self.src)

    def test_both_memcopies_are_tracked(self):
        self.assertIn("mark_dirty(to, b);", self.src)
        self.assertIn("mark_dirty(dst, val);", self.src)

    def test_master_and_runtime_flush_separately(self):
        self.assertIn("begin_dirty(1);", self.src)
        self.assertIn("begin_dirty(0);", self.src)
        self.assertGreaterEqual(self.src.count("flush_dirty();"), 2)
        self.assertIn("sceKernelIcacheInvalidateRange(", self.src)

if __name__ == "__main__":
    unittest.main()
