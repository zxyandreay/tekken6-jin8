"""KiwiYu v0.2 telemetry checks; source-level only, not gameplay validation."""
from pathlib import Path
import unittest
SRC=(Path(__file__).resolve().parents[1]/"plugin-kiwiyu/src/main.c").read_text()
class KiwiV02(unittest.TestCase):
    def test_fail_closed_gate(self):
        self.assertIn('battle_clock_sample(&clock, round_value)', SRC)
        self.assertIn('NO MASTER WRITES: waiting for supported countdown', SRC)
        self.assertIn('ROUND TIMER ACTIVE: battle confirmed', SRC)
    def test_reports_actual_runtime_changes(self):
        self.assertIn('changed_writes++;', SRC)
        self.assertIn('first runtime changed write operations = ', SRC)
    def test_compatible_with_ultrawide_module_identity(self):
        self.assertIn('strcmp(info.name, "Tekken6JinKiwiYu") == 0', SRC)
if __name__=="__main__":unittest.main()
