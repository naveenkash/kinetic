#!/usr/bin/env python3
"""Fails if a data table disagrees with docs/bible/05-core-mechanics.md.

The bible is canonical for every number (project-structure section 4). This check parses
the bible's tables and the few rules it states in prose, and compares each value with the
CSV data tables in Content/Core/Tables. Run from anywhere:

    python3 game/Tools/ci/check_tables.py [--bible PATH] [--tables DIR]
"""
from __future__ import annotations

import argparse
import csv
import math
import re
import sys
from dataclasses import dataclass
from pathlib import Path

REPO = Path(__file__).resolve().parents[3]
DEFAULT_BIBLE = REPO / "docs" / "bible" / "05-core-mechanics.md"
DEFAULT_TABLES = REPO / "game" / "Content" / "Core" / "Tables"

CLASSES = ["Climber", "Scrapper", "Rigger", "Doubler", "Breaker"]
TOLERANCE = 1e-9


@dataclass
class Report:
    mismatches: list
    checked: int = 0


@dataclass(frozen=True)
class Mismatch:
    table: str
    row: str
    column: str
    bible: float
    csv: float
    source: str

    def __str__(self) -> str:
        return (f"{self.table}[{self.row}].{self.column} = {self.csv:g}, "
                f"but bible/05 {self.source} says {self.bible:g}")


# Markdown parsing --------------------------------------------------------------------

def section(text: str, heading: str) -> str:
    """Body of the section whose heading line starts with `heading`, up to the next heading
    of the same or a higher level."""
    lines = text.splitlines()
    for i, line in enumerate(lines):
        match = re.match(r"^(#+)\s+(.*)$", line)
        if match and match.group(2).startswith(heading):
            level = len(match.group(1))
            body = []
            for nxt in lines[i + 1:]:
                deeper = re.match(r"^(#+)\s", nxt)
                if deeper and len(deeper.group(1)) <= level:
                    break
                body.append(nxt)
            return "\n".join(body)
    raise ValueError(f"bible/05 has no section starting with '{heading}'")


def first_table(body: str) -> list[list[str]]:
    rows = [line for line in body.splitlines() if line.strip().startswith("|")]
    if len(rows) < 2:
        raise ValueError("expected a markdown table")
    cells = [[c.strip() for c in row.strip().strip("|").split("|")] for row in rows]
    return [cells[0]] + cells[2:]  # drop the |---| separator


def number(cell: str) -> float:
    """First number in a cell: '800 (1,200 overspin)' -> 800, '−0.05 s' -> -0.05."""
    cleaned = cell.replace(",", "").replace("−", "-").replace("**", "")
    match = re.search(r"-?\d+(?:\.\d+)?", cleaned)
    if not match:
        raise ValueError(f"no number in '{cell}'")
    return float(match.group(0))


def class_rows(table: list[list[str]]) -> dict[str, dict[str, str]]:
    """A table whose header is classes: {row label: {class: cell}}."""
    header = table[0]
    columns = {name: header.index(name) for name in CLASSES}
    return {row[0]: {name: row[i] for name, i in columns.items()} for row in table[1:]}


def prose(text: str, pattern: str, what: str) -> list[float]:
    match = re.search(pattern, text, re.S)
    if not match:
        raise ValueError(f"bible/05 no longer states {what}")
    return [number(g) for g in match.groups()]


# CSV loading -----------------------------------------------------------------------

BOOLEANS = {"true": "1", "false": "0"}


def load_csv(tables: Path, name: str) -> dict[str, dict[str, str]]:
    """Rows by Name; Unreal-style true/false become 1/0 so every value compares as a number."""
    with open(tables / f"{name}.csv", newline="") as handle:
        return {row["Name"]: {k: BOOLEANS.get(v, v) for k, v in row.items()} for row in csv.DictReader(handle)}


# The checks ------------------------------------------------------------------------

def compare(out: Report, table: str, rows: dict, row: str, column: str,
            bible: float, source: str) -> None:
    out.checked += 1
    if row not in rows:
        raise ValueError(f"{table} has no row '{row}'")
    value = rows[row].get(column)
    if value is None or value == "":
        raise ValueError(f"{table}[{row}] has no '{column}'")
    actual = float(value)
    if not math.isfinite(actual) or abs(actual - bible) > TOLERANCE:
        out.mismatches.append(Mismatch(table, row, column, bible, actual, source))


def check_harness(bible: str, tables: Path, out: Report) -> None:
    rows = load_csv(tables, "DT_HarnessClass")
    body_22 = class_rows(first_table(section(bible, "2.2 Harness classes")))
    body_34 = class_rows(first_table(section(bible, "3.4 Class tuning")))
    body_53 = class_rows(first_table(section(bible, "5.3 Foot-hold")))
    mapping = [
        (body_22, "Mass (kg)", "MassKg", 1, "2.2"),
        (body_22, "Run speed (m/s)", "RunSpeed", 1, "2.2"),
        (body_22, "Sprint (m/s)", "SprintSpeed", 1, "2.2"),
        (body_22, "Jump height (m)", "JumpHeight", 1, "2.2"),
        (body_22, "Mantle height (m)", "MantleHeight", 1, "2.2"),
        (body_22, "Air control", "AirControl", 1, "2.2"),
        (body_22, "Crouch speed (m/s)", "CrouchSpeed", 1, "2.2"),
        (body_34, "Line length (m)", "LineLength", 1, "3.4"),
        (body_34, "Pull tension T (N)", "TensionN", 1, "3.4"),
        (body_34, "Jack launch speed V (m/s)", "KickLaunchSpeedV", 1, "3.4"),
        (body_34, "Hang load (kN)", "HangLoadN", 1000, "3.4"),
        (body_34, "Dog speed (m/s)", "DogSpeed", 1, "3.4"),
        (body_34, "Mode switch (s)", "ModeSwitch", 1, "3.4"),
        (body_34, "Set lines", "SetLines", 1, "3.4"),
        (body_34, "Set line life (s)", "SetLineLife", 1, "3.4"),
        (body_34, "Active lines", "ActiveLines", 1, "3.4"),
        (body_53, "Foot-hold (m/s²)", "FootHold", 1, "5.3"),
    ]
    for source_rows, label, column, scale, where in mapping:
        for name in CLASSES:
            compare(out, "DT_HarnessClass", rows, name, column,
                    number(source_rows[label][name]) * scale, f"{where} '{label}'")
    for name in CLASSES:
        recall = body_34["Recall (s)"][name]
        bible_recall = 0.0 if "instant" in recall else number(recall)
        compare(out, "DT_HarnessClass", rows, name, "Recall", bible_recall, "3.4 'Recall (s)'")
        brace = body_22["Brace (see 5.4)"][name]
        compare(out, "DT_HarnessClass", rows, name, "bCanBrace",
                1.0 if brace.startswith("yes") else 0.0, "2.2 'Brace'")

    compare(out, "DT_HarnessClass", rows, "Doubler", "OverspinTensionN",
            number(re.search(r"\((.*?) overspin\)", body_34["Pull tension T (N)"]["Doubler"]).group(1)),
            "3.4 Doubler overspin tension")
    toward, = prose(bible, r"escalates tension from T toward ([\d,]+) N", "the overspin tension (3.7)")
    compare(out, "DT_HarnessClass", rows, "Doubler", "OverspinTensionN", toward, "3.7 overspin tension")
    if not re.search(r"^#+\s+3\.7 Overspin \(Doubler only\)", bible, re.M):
        raise ValueError("bible/05 3.7 no longer says overspin is Doubler only")
    for name in CLASSES:
        if name != "Doubler":  # 3.7 "Doubler only": any other value would give that class overspin
            compare(out, "DT_HarnessClass", rows, name, "OverspinTensionN", 0.0, "3.7 overspin is Doubler only")
    rigger, breaker = prose(bible, r"foot-hold rises to ([\d.]+) m/s² \(Rigger\) or ([\d.]+) m/s² \(Breaker\)",
                            "the braced foot-holds (5.4)")
    compare(out, "DT_HarnessClass", rows, "Rigger", "BracedFootHold", rigger, "5.4 braced foot-hold")
    compare(out, "DT_HarnessClass", rows, "Breaker", "BracedFootHold", breaker, "5.4 braced foot-hold")
    rigger_stagger, = prose(bible, r"staggered\*\* \(0\.6 s loss of control, ([\d.]+) s against a Rigger",
                            "the Rigger Hang-line stagger (3.5)")
    other_stagger, = prose(bible, r"staggered\*\* \(([\d.]+) s loss of control", "the Hang-line stagger (3.5)")
    for name in CLASSES:
        expected = rigger_stagger if name == "Rigger" else other_stagger
        compare(out, "DT_HarnessClass", rows, name, "HangStaggerSeconds", expected, "3.5 Hang-line stagger")
    keep, = prose(bible, r"other classes lose (\d+)% of horizontal speed", "the airborne fire speed loss (2.3)")
    for name in CLASSES:
        expected = 1.0 if name == "Climber" else 1 - keep / 100
        compare(out, "DT_HarnessClass", rows, name, "AirborneFireSpeedKeep", expected, "2.3 airborne fire")
    ladder, breaker_ladder = prose(bible, r"Ladders\*\*: ([\d.]+) m/s climb \(Breaker ([\d.]+) m/s\)", "ladders (2.3)")
    for name in CLASSES:
        compare(out, "DT_HarnessClass", rows, name, "LadderSpeed",
                breaker_ladder if name == "Breaker" else ladder, "2.3 ladders")
    bonus, = prose(bible, r"Scrapper lines move these (\d+)% faster", "the Scrapper small-prop bonus (6.1)")
    for name in CLASSES:
        compare(out, "DT_HarnessClass", rows, name, "SmallPropSpeedMul",
                1 + bonus / 100 if name == "Scrapper" else 1.0, "6.1 Scrapper bonus")


def check_cutter(bible: str, tables: Path, out: Report) -> None:
    rows = load_csv(tables, "DT_CutterVariant")
    table = first_table(section(bible, "4.1 Draw and cut"))
    header = table[0]
    labels = {row[0]: row for row in table[1:]}
    for variant in ["Standard", "Hot", "Long"]:
        col = header.index(variant)
        for label, column in [("Warm-up (draw to cutting)", "Warmup"), ("Swing", "Swing"),
                              ("Recovery", "Recovery"), ("Reach (m)", "Reach")]:
            compare(out, "DT_CutterVariant", rows, variant, column, number(labels[label][col]), f"4.1 '{label}'")
        cone, = prose(bible, r"within a (\d+)° cone", "the Cutter cone (4.2)")
        compare(out, "DT_CutterVariant", rows, variant, "ConeDeg", cone, "4.2 cone")
    harness = load_csv(tables, "DT_HarnessClass")
    scrapper = number(labels["Warm-up (draw to cutting)"][header.index("Scrapper class bonus")])
    breaker, rigger = prose(bible, r"Breaker class penalty: \+([\d.]+) s warm-up\. Rigger class bonus: −([\d.]+) s",
                            "the Breaker and Rigger warm-up offsets (4.1)")
    for name, expected in [("Scrapper", scrapper), ("Breaker", breaker), ("Rigger", -rigger),
                           ("Climber", 0.0), ("Doubler", 0.0)]:
        compare(out, "DT_HarnessClass", harness, name, "CutterWarmupOffset", expected, "4.1 class warm-up offset")


def check_impacts(bible: str, tables: Path, out: Report) -> None:
    rows = load_csv(tables, "DT_ImpactThresholds")
    table = first_table(section(bible, "7.1 Thresholds on a player"))
    labels = {row[0]: row for row in table[1:]}
    mapping = [("Player hits static world", "PlayerVsStatic"), ("Player hits kinematic", "PlayerVsKinematic"),
               ("Object hits player", "ObjectVsPlayer"), ("Player hits another player", "PlayerVsPlayerFloor")]
    for label, row in mapping:
        for i, column in enumerate(["Stagger", "Flip", "Stall"], start=1):
            compare(out, "DT_ImpactThresholds", rows, row, column, number(labels[label][i]), f"7.1 '{label}'")
    crush = labels["Player crushed (pinned between two bodies with closing force)"][3]
    force, seconds = prose(crush, r"force over ([\d,]+) kN for ([\d.]+) s", "the crush rule (7.1)")
    compare(out, "DT_ImpactThresholds", rows, "Crush", "Stall", force * 1000, "7.1 crush force")
    compare(out, "DT_ImpactThresholds", rows, "Crush", "Seconds", seconds, "7.1 crush time")
    stagger, flip = prose(bible, r"moving faster than ([\d.]+) m/s is \*\*staggered.*?Faster than ([\d.]+) m/s, the player is \*\*flipped",
                          "the Hang-line hit speeds (3.5)")
    compare(out, "DT_ImpactThresholds", rows, "HangLineHit", "Stagger", stagger, "3.5 Hang line stagger")
    compare(out, "DT_ImpactThresholds", rows, "HangLineHit", "Flip", flip, "3.5 Hang line flip")
    ledge, = prose(bible, r"falling at under ([\d.]+) m/s", "the ledge grab limit (2.3)")
    compare(out, "DT_ImpactThresholds", rows, "LedgeGrabMax", "Stall", ledge, "2.3 ledge grab")

    surfaces = load_csv(tables, "DT_SurfaceModifiers")
    surface_table = {row[0]: row[1] for row in first_table(section(bible, "7.3 Surface modifiers"))[1:]}
    for label, row in [("Hard (steel, concrete)", "Hard"), ("Wood, planks", "Wood"),
                       ("Scrap heap, laundry, drip nets", "Scrap"), ("Water over 1 m deep", "DeepWater"),
                       ("Stall Gel pad (prep-crew placed, rare)", "StallGel")]:
        compare(out, "DT_SurfaceModifiers", surfaces, row, "Multiplier", number(surface_table[label].replace("×", "")),
                "7.3 surface")


def check_objects(bible: str, tables: Path, out: Report) -> None:
    rows = load_csv(tables, "DT_BreakFree")
    table = {row[0]: row[1] for row in first_table(section(bible, "6.2 Break-free threshold"))[1:]}
    mapping = [("Loose on floor", "Loose"), ("In a rack or cradle", "Rack"), ("In a stack (friction)", "Stack"),
               ("Bolted or chained (removable)", "Bolted"), ("Rated / structural", "Rated"),
               ("Hanging house on its load cable (Hangtown)", "HangingHouse")]
    for label, row in mapping:
        compare(out, "DT_BreakFree", rows, row, "BreakFreeN", number(table[label]), f"6.2 '{label}'")

    tiers = load_csv(tables, "DT_MassTiers")
    tier_table = {re.sub(r"\*", "", row[0]): row[1] for row in first_table(section(bible, "6.1 Mass tiers"))[1:]}
    compare(out, "DT_MassTiers", tiers, "Light", "UpperBoundKg", number(tier_table["Light"]), "6.1 Light")
    compare(out, "DT_MassTiers", tiers, "Medium", "UpperBoundKg",
            float(tier_table["Medium"].split("–")[1].split()[0].replace(",", "")), "6.1 Medium")
    compare(out, "DT_MassTiers", tiers, "Heavy", "UpperBoundKg",
            float(tier_table["Heavy"].split("–")[1].split()[0].replace(",", "")), "6.1 Heavy")

    volatiles = load_csv(tables, "DT_Volatiles")
    vtable = {re.sub(r"\*\*(.*?)\*\*.*", r"\1", row[0]): row for row in first_table(section(bible, "6.4 Volatiles"))[1:]}
    for label, row in [("Propane cylinder", "Propane"), ("Fuel drum", "FuelDrum"), ("Demolition charge", "DemoCharge"),
                       ("Compressed-gas manifold", "GasManifold")]:
        trigger, effect = vtable[label][1], vtable[label][2]
        # Every column the code reads: a value the row does not state must be 0, or the code
        # would act on it (an authored Stall radius, a Jack trigger).
        speed = re.search(r"[Ii]mpact over (\d+) m/s", trigger)
        jack = re.search(r"Jack at or above ([\d,]+) N", trigger)
        radius = re.search(r"[Bb]last radius ([\d.]+) m", effect)
        impulse = re.search(r"[Ii]mpulse ([\d,]+) N·s", effect)
        authored = re.search(r"within ([\d.]+) m are Stalled and players at ([\d.]+)–([\d.]+) m", effect)
        # A fact stated in other words must not read as "not stated" (and so as 0).
        for found, mentioned, what in [(speed, re.search(r"m/s", trigger), "an impact trigger speed"),
                                       (jack, re.search(r"Jack", trigger), "a Jack trigger"),
                                       (radius, re.search(r"radius", effect, re.I), "a blast radius"),
                                       (impulse, re.search(r"N·s", effect), "a blast impulse"),
                                       (authored, re.search(r"stall|flip|scrap", effect, re.I),
                                        "authored Stall and flip radii")]:
            if mentioned and not found:
                raise ValueError(f"bible/05 6.4 {label} states {what} in words the check cannot read")
        stated = lambda m, i=1: number(m.group(i)) if m else 0.0
        compare(out, "DT_Volatiles", volatiles, row, "TriggerSpeed", stated(speed), f"6.4 {label} trigger")
        compare(out, "DT_Volatiles", volatiles, row, "TriggerJackN", stated(jack), f"6.4 {label} trigger")
        compare(out, "DT_Volatiles", volatiles, row, "BlastRadius", stated(radius), f"6.4 {label} radius")
        compare(out, "DT_Volatiles", volatiles, row, "BlastImpulseNs", stated(impulse), f"6.4 {label} impulse")
        compare(out, "DT_Volatiles", volatiles, row, "BlastStallRadius", stated(authored, 1), f"6.4 {label} Stall radius")
        compare(out, "DT_Volatiles", volatiles, row, "BlastFlipRadius", stated(authored, 3), f"6.4 {label} flip radius")
        if authored:
            # The flip band starts where the Stall radius ends (the code flips from there out).
            compare(out, "DT_Volatiles", volatiles, row, "BlastStallRadius", stated(authored, 2),
                    f"6.4 {label} flip band start")
        falloff = re.search(r"falling linearly to 0 at ([\d.]+) m", effect)
        if re.search(r"fall(?:ing|s|off)|linear", effect, re.I) and not falloff:
            raise ValueError(f"bible/05 6.4 {label} states a falloff in words the check cannot read")
        if falloff:
            compare(out, "DT_Volatiles", volatiles, row, "BlastRadius", stated(falloff), f"6.4 {label} falloff")
    if not re.search(r"Pushes, does not kill", vtable["Compressed-gas manifold"][2]):
        raise ValueError("bible/05 6.4 no longer says the manifold does not kill")
    globals_rows = load_csv(tables, "DT_SpoolGlobals")
    compare(out, "DT_SpoolGlobals", globals_rows, "ManifoldJackTrigger", "Value",
            number(re.search(r"Jack at or above ([\d,]+) N", vtable["Compressed-gas manifold"][1]).group(1)),
            "6.4 manifold trigger")


def check_globals(bible: str, tables: Path, out: Report) -> None:
    rows = load_csv(tables, "DT_SpoolGlobals")
    rules = [
        ("HaulRamp", r"ramping from 0 to T over ([\d.]+) s", "3.2 Haul ramp"),
        ("DogsMeet", r"until L ≤ ([\d.]+) m \(the Dogs meet\)", "3.2 Dogs meet"),
        ("HaulCloseCap", r"Maximum closing speed is capped at (\d+) m/s", "3.2 Haul cap"),
        ("KickTime", r"launch speed V\*\* \(class value in 3\.4\) within ([\d.]+) s", "3.2 Kick time"),
        ("InterlockRamp", r"motor push at T only, ramped over ([\d.]+) s", "3.2 interlock ramp"),
        ("JackSeparateCap", r"Maximum separation speed (\d+) m/s", "3.2 Jack cap"),
        ("SlipDistance", r"Slipping extends the line ([\d.]+) m per", "3.2 slip distance"),
        ("SlipInterval", r"Slipping extends the line [\d.]+ m per ([\d.]+) s", "3.2 slip interval"),
        ("ReelRate", r"shortens the rod at ([\d.]+) m/s", "3.2 reel"),
        ("DogRadius", r"gravity, ([\d.]+) m radius", "3.1 Dog radius"),
        ("SlackTimeout", r"The player has (\d+) s to fire B", "3.1 slack timeout"),
        ("DogMissRecall", r"auto-recalls in (\d+) s", "3.1 miss recall"),
        ("SetLineFade", r"recalls over ([\d.]+) s with a visible fade", "3.6 set-line fade"),
        ("OverspinRamp", r"toward [\d,]+ N over ([\d.]+) s", "3.7 overspin ramp"),
        ("OverspinOctave", r"At ([\d.]+) s the line's pitch jumps an octave", "3.7 octave"),
        ("OverspinSnap", r"At ([\d.]+) s the line \*\*snaps", "3.7 snap"),
        ("OverspinLaunch", r"launched opposite the line's direction at (\d+) m/s", "3.7 launch"),
        ("OverspinStagger", r"at \d+ m/s and staggered ([\d.]+) s", "3.7 stagger"),
        ("SnapImpulseBase", r"impulse of \*\*([\d,]+) N·s × \(F / [\d,]+ N\)", "3.7 snap impulse"),
        ("SnapReferenceForce", r"impulse of \*\*[\d,]+ N·s × \(F / ([\d,]+) N\)", "3.7 snap reference"),
        ("OverspinCooldown", r"The line is unavailable for (\d+) s", "3.7 cooldown"),
        ("DoublerSplit", r"each line gets (\d+)% of T", "3.8 split"),
        ("DoublerRestore", r"restores the other to full T over ([\d.]+) s", "3.8 restore"),
        ("CutRecoilPer1000N", r"recoil of (\d+) N·s per 1,000 N", "4.2 cut recoil"),
        ("HardVelocityCap", r"(\d+) m/s is a hard cap on the speed", "6.3 hard cap"),
        ("RatedCommitHold", r"reaches it for ([\d.]+) s on the authored axis", "6.2a commit hold"),
        ("SmallPropBonusMassCap", r"Medium objects under (\d+) kg 30% faster", "6.1 bonus mass cap"),
        ("ForceVolumeReferenceMassKg", r"acceleration a (\d+) kg reference body", "6.6 reference mass"),
        ("ForceVolumeCapMul", r"capped at (\d+) × a_ref", "6.6 cap"),
        ("SetLineProximity", r"brightens within (\d+) m of a player", "11 set line"),
    ]
    for row, pattern, source in rules:
        value, = prose(bible, pattern, source)
        if row == "DoublerSplit":
            value /= 100
        compare(out, "DT_SpoolGlobals", rows, row, "Value", value, source)


CHECKS = [check_harness, check_cutter, check_impacts, check_objects, check_globals]


def run(bible_path: Path, tables: Path) -> Report:
    bible = bible_path.read_text(encoding="utf-8")
    report = Report(mismatches=[])
    for check in CHECKS:
        check(bible, tables, report)
    return report


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("--bible", type=Path, default=DEFAULT_BIBLE)
    parser.add_argument("--tables", type=Path, default=DEFAULT_TABLES)
    args = parser.parse_args(argv)
    try:
        report = run(args.bible, args.tables)
    except (ValueError, KeyError, FileNotFoundError) as error:
        print(f"check_tables: cannot check: {error}", file=sys.stderr)
        return 2
    for mismatch in report.mismatches:
        print(f"MISMATCH {mismatch}", file=sys.stderr)
    status = "FAIL" if report.mismatches else "ok"
    print(f"check_tables: {status}, {report.checked} values checked, {len(report.mismatches)} mismatches")
    return 1 if report.mismatches else 0


if __name__ == "__main__":
    sys.exit(main())
