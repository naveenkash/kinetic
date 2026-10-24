"""Tests for check_tables.py. Run: python3 -m unittest discover game/Tools/ci"""
import shutil
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import check_tables  # noqa: E402


class CheckTablesTest(unittest.TestCase):
    def setUp(self):
        self.work = Path(tempfile.mkdtemp())
        self.tables = self.work / "Tables"
        shutil.copytree(check_tables.DEFAULT_TABLES, self.tables)
        self.bible = self.work / "05.md"
        shutil.copy(check_tables.DEFAULT_BIBLE, self.bible)

    def tearDown(self):
        shutil.rmtree(self.work)

    def replace_in(self, path: Path, old: str, new: str):
        text = path.read_text(encoding="utf-8")
        self.assertEqual(text.count(old), 1, f"fixture text not unique: {old}")
        path.write_text(text.replace(old, new, 1), encoding="utf-8")

    def test_current_tables_match_the_bible(self):
        report = check_tables.run(self.bible, self.tables)
        self.assertEqual(report.mismatches, [])
        self.assertGreater(report.checked, 200)

    def test_a_changed_class_value_is_reported(self):
        self.replace_in(self.tables / "DT_HarnessClass.csv", "Breaker,220,", "Breaker,200,")
        report = check_tables.run(self.bible, self.tables)
        self.assertEqual([(m.row, m.column) for m in report.mismatches], [("Breaker", "MassKg")])

    def test_a_changed_bible_number_is_reported(self):
        self.replace_in(self.bible, "| Pull tension T (N) | 400 | 400 | 600 |", "| Pull tension T (N) | 400 | 400 | 650 |")
        report = check_tables.run(self.bible, self.tables)
        self.assertEqual([(m.row, m.column) for m in report.mismatches], [("Rigger", "TensionN")])

    def test_a_changed_prose_rule_is_reported(self):
        self.replace_in(self.bible, "ramping from 0 to T over 0.4 s", "ramping from 0 to T over 0.5 s")
        report = check_tables.run(self.bible, self.tables)
        self.assertEqual([(m.row, m.column) for m in report.mismatches], [("HaulRamp", "Value")])

    def mismatched(self):
        return [(m.row, m.column) for m in check_tables.run(self.bible, self.tables).mismatches]

    def test_a_non_finite_value_is_reported(self):
        self.replace_in(self.tables / "DT_SurfaceModifiers.csv", "StallGel,4.0", "StallGel,nan")
        self.assertEqual(self.mismatched(), [("StallGel", "Multiplier")])

    def test_overspin_on_another_class_is_reported(self):
        self.replace_in(self.tables / "DT_HarnessClass.csv", ",40,600,0,12,", ",40,600,1200,12,")
        self.assertEqual(self.mismatched(), [("Rigger", "OverspinTensionN")])

    def test_an_unstated_blast_column_is_reported(self):
        self.replace_in(self.tables / "DT_Volatiles.csv", "DemoCharge,6,0,5,2500,0,0", "DemoCharge,6,0,5,2500,5,5")
        self.assertEqual(self.mismatched(), [("DemoCharge", "BlastStallRadius"), ("DemoCharge", "BlastFlipRadius")])

    def test_a_moved_propane_flip_band_is_reported(self):
        self.replace_in(self.bible, "players at 2–4 m are flipped", "players at 3–4 m are flipped")
        self.assertEqual(self.mismatched(), [("Propane", "BlastStallRadius")])

    def test_the_overspin_ramp_and_interlock_ramp_are_their_own_rules(self):
        self.replace_in(self.bible, "toward 1,200 N over 1.0 s", "toward 1,200 N over 1.5 s")
        self.replace_in(self.bible, "motor push at T only, ramped over 0.3 s", "motor push at T only, ramped over 0.4 s")
        self.assertEqual(self.mismatched(), [("InterlockRamp", "Value"), ("OverspinRamp", "Value")])

    def test_a_volatile_fact_in_other_words_fails_loudly(self):
        for old, new in [("Breaks the structural element", "Players within 3 m are Stalled. Breaks the structural element"),
                         ("Breaks the structural element", "Within 3 m: a Stall; to 5 m: a flip. Breaks the structural element"),
                         ("Breaks the structural element", "Players within 3 m are scrapped. Breaks the structural element"),
                         ("falling linearly to 0 at 4 m", "falling linearly to nothing at 4.5 m"),
                         ("Pushes, does not kill.", "Pushes, does not kill. Blasts for 900 N·s within 2 m.")]:
            self.tearDown()
            self.setUp()  # A fresh copy for each case
            self.replace_in(self.bible, old, new)
            self.assertEqual(check_tables.main(["--bible", str(self.bible), "--tables", str(self.tables)]), 2)

    def test_decimal_authored_radii_are_read(self):
        self.replace_in(self.bible, "Players within 2 m are Stalled and players at 2–4 m",
                        "Players within 2.5 m are Stalled and players at 2.5–4 m")
        self.assertEqual(self.mismatched(), [("Propane", "BlastStallRadius"), ("Propane", "BlastStallRadius")])

    def test_a_missing_bible_section_fails_loudly(self):
        self.replace_in(self.bible, "### 5.3 Foot-hold", "### 5.3 Footing")
        self.assertEqual(check_tables.main(["--bible", str(self.bible), "--tables", str(self.tables)]), 2)


if __name__ == "__main__":
    unittest.main()
