#!/usr/bin/env python3
"""Compare reference output with C, Rust, or official-style 1brc output."""

import argparse
from pathlib import Path
import re
import sys


NUMBER = r"-?[0-9]+\.[0-9]"
STATS = rf"({NUMBER})/({NUMBER})/({NUMBER})"
LINE = re.compile(rf"(.+)[:=]{STATS}")
MAP_ENTRY = re.compile(rf"([^\n]+?)[:=]{STATS}(?=, |\Z)")


def tenths(text):
    # Exact comparison; -0.0 and 0.0 represent the same temperature.
    return int(text.replace(".", ""))


def load(path):
    text = path.read_text(encoding="utf-8")
    entries = {}

    def add(match):
        city, *values = match.groups()
        if city in entries:
            raise ValueError(f"duplicate city {city!r}")
        stats = tuple(tenths(value) for value in values)
        if not -999 <= stats[0] <= stats[1] <= stats[2] <= 999:
            raise ValueError(f"invalid min/mean/max for {city!r}")
        entries[city] = stats

    # Detect a complete wrapped map, preserving whitespace in city names.
    wrapped = text.rstrip("\n")
    if wrapped.startswith("{") and wrapped.endswith("}"):
        body = wrapped[1:-1]
        position = 0
        while position < len(body):
            match = MAP_ENTRY.match(body, position)
            if match is None:
                raise ValueError(f"malformed map near character {position + 1}")
            add(match)
            position = match.end()
            if position < len(body):
                position += 2  # Consume ', '; allow Rust's trailing separator.
    else:
        lines = text.split("\n")
        for number, line in enumerate(lines, 1):
            if line == "" and number == len(lines):
                continue  # Empty file or final newline.
            match = LINE.fullmatch(line)
            if match is None:
                raise ValueError(f"malformed output on line {number}")
            add(match)
    return entries


def display(stats):
    def number(value):
        return f"{'-' if value < 0 else ''}{abs(value) // 10}.{abs(value) % 10}"
    return "/".join(number(value) for value in stats)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("expected", type=Path)
    parser.add_argument("actual", type=Path)
    parser.add_argument("--limit", type=int, default=20,
                        help="maximum number of differences to show (default: 20)")
    args = parser.parse_args()
    if args.limit < 0:
        parser.error("--limit must be nonnegative")

    outputs = []
    for path in (args.expected, args.actual):
        try:
            outputs.append(load(path))
        except (OSError, UnicodeError, ValueError) as error:
            print(f"ERROR: {path}: {error}", file=sys.stderr)
            return 2
    expected, actual = outputs
    missing = sorted(expected.keys() - actual.keys())
    extra = sorted(actual.keys() - expected.keys())
    changed = sorted(city for city in expected.keys() & actual.keys()
                     if expected[city] != actual[city])
    if not (missing or extra or changed):
        print(f"PASS: all {len(expected)} cities match")
        return 0

    print(f"FAIL: {len(missing)} missing, {len(extra)} extra, "
          f"{len(changed)} differing cities")
    shown = 0
    for kind, cities in (("missing", missing), ("extra", extra), ("different", changed)):
        for city in cities:
            if shown >= args.limit:
                break
            if kind == "different":
                print(f"  {city!r}: expected {display(expected[city])}, "
                      f"got {display(actual[city])}")
            else:
                print(f"  {kind}: {city!r}")
            shown += 1
    remaining = len(missing) + len(extra) + len(changed) - shown
    if remaining:
        print(f"  ... {remaining} more differences")
    return 1


if __name__ == "__main__":
    sys.exit(main())
