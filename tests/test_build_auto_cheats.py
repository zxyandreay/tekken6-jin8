import unittest
from tools.build_auto_cheats import split_master, safe_boundaries, GATE_OFFSET, LAST_MASTER_OFFSET


class TestAutoCheats(unittest.TestCase):
    def test_chunks_never_split_e_skip_window(self):
        source = [(0x20000000 | i, i) for i in range(11000)]
        source[3400] = (0xE1000001, 0x004078B4)  # skip 0x100 lines
        chunks = split_master(source, 3800)
        self.assertGreater(len(chunks), 2)
        for i in range(len(chunks)-1):
            self.assertNotEqual(chunks[i][1], 3401)
        self.assertTrue(all(b-a <= 3800 for a,b in chunks))
        self.assertEqual(chunks[0][0], 0)
        self.assertEqual(chunks[-1][1], len(source))

    def test_gate_memory_is_beyond_last_master_write(self):
        self.assertEqual(0x08800000 + GATE_OFFSET, 0x0A1FFFC0)
        self.assertEqual(0x08800000 + LAST_MASTER_OFFSET, 0x0A1F0AB0)
        self.assertGreater(GATE_OFFSET, LAST_MASTER_OFFSET)
