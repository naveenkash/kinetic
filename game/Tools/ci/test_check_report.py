"""Tests for check_report.py. Run: python3 -m unittest discover game/Tools/ci"""
import copy
import hashlib
import json
import shutil
import sys
import tempfile
import unittest
from pathlib import Path

sys.path.insert(0, str(Path(__file__).parent))
import check_report  # noqa: E402

EVENTS = '{"type":"round_end","v":1,"t":60,"round":0,"winner_team":1,"seconds":60,"stalls":3,"reason":"crew_down"}\n'
HUMAN = "7d1b2c3a-0000-4000-8000-000000000000"


def facts():
    return {"mass_distance_kg_m": 0, "eliminations": 0, "assists": 0, "spotter_credits": 0,
            "ringout_heights_m": [], "rope_marks": [],
            "counters": {name: 0 for name in check_report.COUNTERS},
            "floor_seconds": 10, "airborne_seconds": 0, "hoisted_seconds": 0}


def report():
    return {
        "match_id": "5b0f6a3e-9c1d-4e2f-8a7b-1c2d3e4f5a6b", "build": "0.1.0", "ruleset": 1, "event_schema": 1,
        "mode": "scrap", "arena": "YRD", "variant": "base", "winner_team": 2,
        "rounds": [{"winner_team": 2, "seconds": 60, "stalls": 3, "end": "crew_down"}],
        "players": [
            {"account_id": HUMAN, "team": 1, "harness": "Rigger", "face": None, "connected_seconds": 60,
             "abandoned": False, "is_bot": False, "facts": facts()},
            {"account_id": None, "team": 2, "harness": "Breaker", "face": None, "connected_seconds": 60,
             "abandoned": False, "is_bot": True, "facts": facts()},
        ],
        "raw_events_sha256": hashlib.sha256(EVENTS.encode()).hexdigest(),
    }


class CheckReportTest(unittest.TestCase):
    def setUp(self):
        self.work = Path(tempfile.mkdtemp())
        self.events = self.work / "events.jsonl"
        self.events.write_text(EVENTS, encoding="utf-8")

    def tearDown(self):
        shutil.rmtree(self.work)

    def problems(self, data, events=None):
        path = self.work / "report.json"
        path.write_text(json.dumps(data), encoding="utf-8")
        if events is not None:
            self.events.write_text(events, encoding="utf-8")
        return check_report.run(path, self.events)

    def test_a_good_report_passes(self):
        self.assertEqual(self.problems(report()), [])

    def test_what_the_backend_would_quarantine_is_reported(self):
        cases = [
            (lambda r: r["players"][0].update(team=3), "team"),
            (lambda r: r["players"][0].update(account_id="not-a-uuid"), "account_id"),
            (lambda r: r["players"][0].update(is_bot="false"), "is_bot"),
            (lambda r: r["players"][0].update(connected_seconds=-1), "connected_seconds"),
            (lambda r: r["players"].append(copy.deepcopy(r["players"][0])), "twice"),
            (lambda r: r.update(winner_team=0), "winner_team"),
            (lambda r: r.update(match_id="m-1"), "match_id"),
            (lambda r: r["players"][1]["facts"]["counters"].pop("snaps_survived"), "snaps_survived"),
            (lambda r: r["players"][0]["facts"].update(floor_seconds=61), "more than the time connected"),
            (lambda r: r.update(match_id=r["match_id"] + "\n"), "match_id"),
            (lambda r: r["players"][0].update(account_id=HUMAN + "\n"), "account_id"),
            (lambda r: r["players"][0].update(face="a\x00b"), "jsonb"),
            (lambda r: r["players"][0].update(face="\ud800"), "jsonb"),
            (lambda r: r.update(padding="x" * 300000), "256 KB"),
            (lambda r: r["players"][0]["facts"].update(counters=[1]), "snaps_survived"),
            (lambda r: r["players"][0].update(account_id=[1]), "account_id"),
            (lambda r: r["players"][0].update(connected_seconds=10 ** 400), "connected_seconds"),
        ]
        for change, expected in cases:
            data = report()
            change(data)
            found = self.problems(data)
            self.assertTrue(any(expected in p for p in found), f"{expected}: {found}")

    def test_what_the_backend_accepts_passes(self):
        data = report()
        data["players"][0]["account_id"] = HUMAN.upper()
        del data["players"][0]["abandoned"]
        data["players"][1]["is_bot"] = True
        data["winner_team"] = 1.5
        self.assertEqual(self.problems(data), [])

    def test_nan_and_bad_bytes_are_problems_not_crashes(self):
        path = self.work / "report.json"
        path.write_text(json.dumps(report()).replace('"seconds": 60', '"seconds": NaN', 1), encoding="utf-8")
        self.assertTrue(check_report.run(path, self.events)[0].startswith("cannot read the report"))
        path.write_bytes(b"\xff")
        self.assertTrue(check_report.run(path, self.events)[0].startswith("cannot read the report"))
        for line in ("[1,2]", "7", '{"type":"round_end","v":1,"t":NaN}'):
            data = report()
            text = line + "\n"
            data["raw_events_sha256"] = hashlib.sha256(text.encode()).hexdigest()
            self.assertTrue(self.problems(data, text))
        data = report()
        text = '{"type":"round_end","v":1,"note":"a\u2028b"}\n'
        data["raw_events_sha256"] = hashlib.sha256(text.encode()).hexdigest()
        self.assertEqual(self.problems(data, text), [])  # U+2028 inside a string is one line

    def test_size_is_measured_as_javascript_writes_it(self):
        cases = {60.0: "60", 1e20: "100000000000000000000", 1e21: "1e+21", 1.5e-7: "1.5e-7", 0.000001: "0.000001",
                 -2.5: "-2.5", 123.456: "123.456", 5e-324: "5e-324", 0.1: "0.1", 1e16: "10000000000000000"}
        for value, text in cases.items():
            self.assertEqual(check_report.js_number(value), text)
        big = report()
        big["padding"] = [1e20] * 16000  # Short in Python, 21 characters each in JavaScript
        self.assertTrue(any("256 KB" in p for p in self.problems(big)))
        small = report()
        small["padding"] = [60.0] * 60000  # "60.0" in Python, "60" in JavaScript: under the limit
        self.assertFalse(any("256 KB" in p for p in self.problems(small)))

    def test_deep_nesting_is_a_problem_not_a_crash(self):
        path = self.work / "report.json"
        path.write_text("[" * 100000 + "]" * 100000, encoding="utf-8")
        self.assertTrue(check_report.run(path, self.events)[0].startswith("cannot read the report"))
        data = report()
        deep = "[" * 100000 + "]" * 100000 + "\n"
        data["raw_events_sha256"] = hashlib.sha256(deep.encode()).hexdigest()
        self.assertTrue(any("not JSON" in p for p in self.problems(data, deep)))

    def test_the_events_must_match_their_digest_and_schema(self):
        self.assertTrue(any("sha256" in p for p in self.problems(report(), EVENTS + EVENTS)))
        bad = '{"type":"teleport","v":1}\n'
        data = report()
        data["raw_events_sha256"] = hashlib.sha256(bad.encode()).hexdigest()
        self.assertTrue(any("unknown type" in p for p in self.problems(data, bad)))
        broken = "{not json\n"
        data["raw_events_sha256"] = hashlib.sha256(broken.encode()).hexdigest()
        self.assertTrue(any("not JSON" in p for p in self.problems(data, broken)))


if __name__ == "__main__":
    unittest.main()
