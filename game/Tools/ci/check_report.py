#!/usr/bin/env python3
"""Checks a match report and its raw events the way the backend will read them.

The report must be JSON the server-report function can parse (no NaN or Infinity), no larger
than its 256 KB limit, storable as Postgres jsonb (no \u0000, no lone surrogates), and accepted
by srv_store_report (backend migration 20261024000200_functions.sql): a players array of
objects; is_bot and abandoned booleans, or missing or null (false); connected_seconds a number
from 0; team 1 or 2; a UUID account_id for every human (any case), or null for an account
deleted before the manifest, and no human twice;
winner_team null or a number from 1 to 2. The raw events must be JSON Lines on the same schema,
and their SHA-256 must be the report's raw_events_sha256.

    python3 game/Tools/ci/check_report.py REPORT.json EVENTS.jsonl
"""
from __future__ import annotations

import hashlib
import json
import math
import re
import sys
from decimal import Decimal
from pathlib import Path

EVENT_SCHEMA = 1
RULESET = 1
TEAMS = 2
UUID = re.compile(r"[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}", re.IGNORECASE)
MAX_REPORT_UNITS = 256 * 1024  # server-report: JSON.stringify(body).length, in UTF-16 code units
FACTS = ["mass_distance_kg_m", "eliminations", "assists", "spotter_credits", "ringout_heights_m",
         "rope_marks", "counters", "floor_seconds", "airborne_seconds", "hoisted_seconds"]
COUNTERS = ["lines_hung_used_by_others", "rated_tons_moved", "rounds_without_floor",
            "scraps_with_lighter_object", "snaps_survived"]
EVENT_TYPES = {"dog_seat", "mode", "cut", "stall", "hit", "snap", "rated_commit", "player_round",
               "round_end", "match_end", "connection", "server_second"}


def is_number(value) -> bool:
    if isinstance(value, bool) or not isinstance(value, (int, float)):
        return False
    try:
        return math.isfinite(value)
    except OverflowError:  # An integer beyond any double
        return False


def is_uuid(value) -> bool:
    return isinstance(value, str) and UUID.fullmatch(value) is not None


def is_bool_or_default(value) -> bool:
    return value is None or isinstance(value, bool)  # private.json_bool defaults null and missing


def js_number(value: float) -> str:
    """A number as JavaScript's JSON.stringify writes it (Number::toString), from the same
    shortest round-trip digits Python's repr finds."""
    if not math.isfinite(value):
        return "null"
    if value == 0:
        return "0"
    sign, digits, exponent = Decimal(repr(abs(value))).as_tuple()
    text = "".join(map(str, digits)).rstrip("0")
    point = len(digits) + exponent  # The value is 0.text x 10^point
    k = len(text)
    if k <= point <= 21:
        body = text + "0" * (point - k)
    elif 0 < point <= 21:
        body = text[:point] + "." + text[point:]
    elif -6 < point <= 0:
        body = "0." + "0" * -point + text
    else:
        e = point - 1
        body = text[0] + ("." + text[1:] if k > 1 else "") + "e" + ("+" if e >= 0 else "-") + str(abs(e))
    return ("-" if value < 0 else "") + body


def js_length(value) -> int:
    """len(JSON.stringify(value)) in Deno: UTF-16 code units, numbers in JavaScript form."""
    if value is None:
        return 4
    if isinstance(value, bool):
        return 4 if value else 5
    if isinstance(value, (int, float)):
        try:
            return len(js_number(float(value)))
        except OverflowError:  # Too large for a double: JavaScript reads Infinity
            return 4
    if isinstance(value, str):
        text = json.dumps(value, ensure_ascii=False)
        lone = sum(1 for c in value if 0xD800 <= ord(c) <= 0xDFFF)
        return len(text.encode("utf-16-le", "surrogatepass")) // 2 + 5 * lone  # JS writes \udXXX
    if isinstance(value, list):
        return 2 + sum(js_length(item) for item in value) + max(len(value) - 1, 0)
    if isinstance(value, dict):
        parts = sum(js_length(key) + 1 + js_length(item) for key, item in value.items())
        return 2 + parts + max(len(value) - 1, 0)
    return 0


def refuse_constant(name: str):
    raise ValueError(f"{name} is not JSON")


def bad_strings(value, path: str = "$"):
    """Strings Postgres jsonb cannot store: NUL and unpaired surrogates."""
    if isinstance(value, str):
        if "\x00" in value or any(0xD800 <= ord(c) <= 0xDFFF for c in value):
            yield path
    elif isinstance(value, dict):
        for key, item in value.items():
            yield from bad_strings(key, path + ".<key>")
            yield from bad_strings(item, f"{path}.{key}")
    elif isinstance(value, list):
        for index, item in enumerate(value):
            yield from bad_strings(item, f"{path}[{index}]")


def check_player(index: int, player, problems: list[str]) -> None:
    where = f"players[{index}]"
    if not isinstance(player, dict):
        problems.append(f"{where} is not an object")
        return
    for key in ("is_bot", "abandoned"):
        if not is_bool_or_default(player.get(key)):
            problems.append(f"{where}.{key} is not a boolean")
    if not is_number(player.get("connected_seconds")) or player["connected_seconds"] < 0:
        problems.append(f"{where}.connected_seconds is not a non-negative number")
    if not is_number(player.get("team")) or player["team"] not in range(1, TEAMS + 1):
        problems.append(f"{where}.team is not 1 or 2")
    account = player.get("account_id")
    if player.get("is_bot") is not True and account is not None and not is_uuid(account):
        # Null is a human whose account was deleted before the manifest (migration 20261025000100).
        problems.append(f"{where}.account_id is not a UUID or null for a human")
    facts = player.get("facts")
    if not isinstance(facts, dict):
        problems.append(f"{where}.facts is missing")
        return
    for key in FACTS:
        if key not in facts:
            problems.append(f"{where}.facts.{key} is missing")
    counters = facts.get("counters")
    for key in COUNTERS:
        if not isinstance(counters, dict) or not is_number(counters.get(key)):
            problems.append(f"{where}.facts.counters.{key} is not a number")
    connected = player.get("connected_seconds", 0)
    for key in ("floor_seconds", "airborne_seconds", "hoisted_seconds"):
        if is_number(facts.get(key)) and is_number(connected) and facts[key] > connected + 0.05:
            problems.append(f"{where}.facts.{key} is more than the time connected")


def check_report(report, problems: list[str]) -> None:
    if not isinstance(report, dict):
        problems.append("the report is not an object")
        return
    if not is_uuid(report.get("match_id")):
        problems.append("match_id is not a UUID")
    if report.get("event_schema") != EVENT_SCHEMA or report.get("ruleset") != RULESET:
        problems.append("event_schema or ruleset is not 1")
    winner = report.get("winner_team")
    if winner is not None and (not is_number(winner) or not 1 <= winner <= TEAMS):
        problems.append("winner_team is not null or a number from 1 to 2")
    players = report.get("players")
    if not isinstance(players, list):
        problems.append("players is not an array")
        return
    for index, player in enumerate(players):
        check_player(index, player, problems)
    humans = [p["account_id"].lower() for p in players
              if isinstance(p, dict) and p.get("is_bot") is not True and is_uuid(p.get("account_id"))]
    if len(humans) != len(set(humans)):
        problems.append("a human is reported twice")
    if not isinstance(report.get("rounds"), list):
        problems.append("rounds is not an array")


def check_events(text: str, problems: list[str]) -> None:
    if text and not text.endswith("\n"):
        problems.append("the raw events do not end in a newline")
    lines = text.split("\n")[:-1] if text.endswith("\n") else text.split("\n")
    for number, line in enumerate(lines, start=1):
        try:
            event = json.loads(line, parse_constant=refuse_constant)
        except (ValueError, RecursionError) as error:
            problems.append(f"raw events line {number} is not JSON: {error}")
            continue
        if not isinstance(event, dict) or event.get("v") != EVENT_SCHEMA or event.get("type") not in EVENT_TYPES:
            problems.append(f"raw events line {number} has an unknown type or schema")


def run(report_path: Path, events_path: Path) -> list[str]:
    problems: list[str] = []
    try:
        report_text = report_path.read_bytes().decode("utf-8")
        report = json.loads(report_text, parse_constant=refuse_constant)
    except (OSError, UnicodeDecodeError, ValueError, RecursionError) as error:
        return [f"cannot read the report: {type(error).__name__}: {error}"]
    try:
        events = events_path.read_bytes()
        events_text = events.decode("utf-8")
    except (OSError, UnicodeDecodeError) as error:
        return [f"cannot read the raw events: {error}"]
    try:
        units = js_length(report)
        if units > MAX_REPORT_UNITS:
            problems.append(f"the report is {units} UTF-16 units, over the 256 KB server-report limit")
        for path in bad_strings(report):
            problems.append(f"{path} holds a string Postgres jsonb cannot store")
        check_report(report, problems)
        check_events(events_text, problems)
    except RecursionError:
        problems.append("the report or an event is nested too deeply to check")
    if isinstance(report, dict) and report.get("raw_events_sha256") != hashlib.sha256(events).hexdigest():
        problems.append("raw_events_sha256 is not the digest of the raw events")
    return problems


def main(argv: list[str] | None = None) -> int:
    args = sys.argv[1:] if argv is None else argv
    if len(args) != 2:
        print("usage: check_report.py REPORT.json EVENTS.jsonl", file=sys.stderr)
        return 2
    problems = run(Path(args[0]), Path(args[1]))
    for problem in problems:
        print(f"REPORT {problem}", file=sys.stderr)
    print(f"check_report: {'FAIL' if problems else 'ok'}, {len(problems)} problems")
    return 1 if problems else 0


if __name__ == "__main__":
    sys.exit(main())
