import importlib.util
from pathlib import Path
import tempfile
import unittest

SCRIPT = Path(__file__).resolve().parents[1] / "tools" / "cheat_inventory.py"
spec = importlib.util.spec_from_file_location("cheat_inventory", SCRIPT)
module = importlib.util.module_from_spec(spec)
spec.loader.exec_module(module)


class InventoryTests(unittest.TestCase):
    def test_group_count_and_opcode_family(self):
        with tempfile.TemporaryDirectory() as d:
            path = Path(d) / "test.ini"
            path.write_text(
                "_S ULUS-10466\n_C0 (m)Tekken 8\n_L 0x20910000 0x12345678\n"
                "_C1 (1)jin\n_L 0xE01D0009 0x004078B4\n_L 0x604078E0 0x00000001\n",
                encoding="ascii",
            )
            result = module.summarize(path)
            self.assertEqual(result["totalRecords"], 3)
            self.assertEqual(result["unparsedRecords"], 0)
            self.assertFalse(result["groups"]["(m)Tekken 8"]["enabledAtStart"])
            self.assertTrue(result["groups"]["(1)jin"]["enabledAtStart"])
            self.assertEqual(result["groups"]["(1)jin"]["families"]["0xE"], 1)
            self.assertEqual(result["groups"]["(1)jin"]["families"]["0x6"], 1)

    def test_malformed_line_recorded(self):
        with tempfile.TemporaryDirectory() as d:
            path = Path(d) / "bad.ini"
            path.write_text("_C0 bad\n_L nope\n", encoding="ascii")
            self.assertEqual(module.summarize(path)["unparsedRecords"], 1)


if __name__ == "__main__":
    unittest.main()
