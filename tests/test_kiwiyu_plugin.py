"""Static sanity checks for separate KiwiYu standalone plugin."""
from pathlib import Path
import unittest
SOURCE=(Path(__file__).resolve().parents[1]/"plugin-kiwiyu/src/main.c").read_text()
class KiwiYuTest(unittest.TestCase):
 def test_standalone_name_matches_module_scan(self):
  self.assertIn('PSP_MODULE_INFO("Tekken6JinKiwiYu", 0, 1, 0);',SOURCE)
  self.assertIn('strcmp(info.name, "Tekken6JinKiwiYu") == 0',SOURCE)
 def test_both_masters_required(self):
  self.assertIn('MASTER_LINES 436692u',SOURCE)
  self.assertIn('MASTER2_LINES 437310u',SOURCE)
  self.assertIn('RUNTIME_LINES 5113u',SOURCE)
  self.assertIn('load_master(MASTER_PATH, MASTER_LINES)',SOURCE)
  self.assertIn('load_master(MASTER2_PATH, MASTER2_LINES)',SOURCE)
 def test_independent_and_d_opcode(self):
  self.assertIn('case 13:',SOURCE)
  self.assertIn('sceKernelDelayThread(16000);',SOURCE)
  self.assertIn('ROUND TIMER ACTIVE',SOURCE)
  self.assertNotIn('ms0:/PSP/CHEATS',SOURCE.upper())
if __name__=="__main__": unittest.main()
