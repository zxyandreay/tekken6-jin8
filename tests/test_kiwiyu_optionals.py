"""Source structure test for opt-in KiwiYu + Ultrawide cheat generator.
The original private Kiwi Yu files are not required to run these tests.
"""
from pathlib import Path
import unittest
import importlib.util

P=Path(__file__).resolve().parents[1]/"tools/build_kiwiyu_optionals.py"
spec=importlib.util.spec_from_file_location("kiwi_optionals",P)
mod=importlib.util.module_from_spec(spec)
spec.loader.exec_module(mod)

class OptionalsTest(unittest.TestCase):
    def test_expected_camera_code_values(self):
        self.assertEqual(list(mod.CAMERAS.values()),
            ["0x00003F80", "0x00003F81", "0x00003F82", "0x00003F83"])
    def test_no_core_groups_in_optional_selection(self):
        self.assertEqual(len(mod.KIWI_OPTIONAL), 4)
        self.assertNotIn("JIN Moves", mod.KIWI_OPTIONAL)
        self.assertNotIn("JIN MASTER CODES", mod.KIWI_OPTIONAL)
        self.assertNotIn("TEKKEN 8 MASTER CODES", mod.KIWI_OPTIONAL)
    def test_group_parsing_preserves_disabled_status(self):
        result=mod.groups_from("_S ULUS-10466\n_C0 Turn off Hazards\n_L 0x209400F0 0x00000001\n")
        self.assertEqual(result["Turn off Hazards"][0], "_C0 Turn off Hazards")

if __name__ == "__main__":
    unittest.main()
