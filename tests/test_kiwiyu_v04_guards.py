"""KiwiYu v0.4 source guard regression tests (no proprietary mod bytes)."""
import unittest
from pathlib import Path
SRC = (Path(__file__).resolve().parents[1]/"plugin-kiwiyu/src/main.c").read_text()
class GuardRegression(unittest.TestCase):
    def test_no_runtime_full_range_fallback(self):
        self.assertIn('dirty_pages_add(dirty_page_map, addr, bytes)', SRC)
        self.assertIn('dirty_pages_has(dirty_page_map, p)', SRC)
        self.assertNotIn('dirty_overflow', SRC)
    def test_85_mib_memcpy_opt_in(self):
        self.assertIn('b > LARGE_COPY_LIMIT && !allow_large_copies', SRC)
        self.assertIn('a == 0x08C07D24u && size == 4 && val == 0x12345678u', SRC)
    def test_guard_already_equal_short_memcpy(self):
        self.assertIn('if (memcmp((const void *)(uintptr_t)to, (const void *)(uintptr_t)a, b) != 0)', SRC)
    def test_no_reapply_after_one_second_gap(self):
        self.assertIn('idle_cycles == 320 && installed', SRC)
        self.assertNotIn('base != prior_base', SRC)
        self.assertIn('total master initializations = ', SRC)
    def test_preserve_original_cinematic_opt_in(self):
        self.assertIn('KiwiYu.cinematics', SRC)
        self.assertIn('KiwiYu.allowlargecopies', SRC)
        self.assertIn('0x0895350Cu || a == 0x08D6F270u', SRC)
if __name__=="__main__": unittest.main()
