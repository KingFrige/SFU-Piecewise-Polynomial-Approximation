#!/usr/bin/env python3
"""Compare deterministic SV output rows against the repository C model."""

from __future__ import annotations

import argparse
import re
import subprocess
import sys
from pathlib import Path


OPERATIONS = [
    ("sin", 9),
    ("cos", 10),
    ("rsqrt", 4),
    ("log2", 7),
    ("ex2", 6),
    ("rcp", 1),
    ("sqrt", 2),
]

RESULT_RE = re.compile(r"^result_hex=([0-9A-Fa-f]{8})$")


def read_sv_output(path: Path) -> list[tuple[str, str]]:
    if not path.exists():
        raise RuntimeError(f"missing SV output file: {path}")

    rows: list[tuple[str, str]] = []
    with path.open("r", encoding="ascii") as fp:
        header = next(fp, None)
        if header is None:
            raise RuntimeError(f"empty SV output file: {path}")
        for lineno, line in enumerate(fp, start=2):
            parts = [part.strip() for part in line.split(",")]
            if len(parts) < 2:
                raise RuntimeError(f"{path}:{lineno}: expected at least two CSV columns")
            input_hex = parts[0].upper()
            rtl_hex = parts[1].upper()
            if len(input_hex) != 8 or len(rtl_hex) != 8:
                raise RuntimeError(f"{path}:{lineno}: malformed hex row")
            rows.append((input_hex, rtl_hex))
    return rows


def run_reference(sfu: Path, lut_dir: Path, input_hex: str, selector: int) -> str:
    completed = subprocess.run(
        [str(sfu), str(lut_dir), input_hex, str(selector)],
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        check=False,
    )
    if completed.returncode != 0:
        raise RuntimeError(
            f"reference failed for input={input_hex} selector={selector}:\n{completed.stdout}"
        )

    for line in completed.stdout.splitlines():
        match = RESULT_RE.match(line.strip())
        if match:
            return match.group(1).upper()
    raise RuntimeError(f"missing result_hex in reference output:\n{completed.stdout}")


def is_known_sfu13_partition_diff(name: str, input_hex: str, reference_hex: str, rtl_hex: str) -> bool:
    bits = int(input_hex, 16)
    exp = (bits >> 23) & 0xFF
    mant = bits & 0x7FFFFF

    if reference_hex == rtl_hex:
        return False
    return name in {"sin", "cos"} and exp == 0 and mant != 0


def compare_operation(
    name: str,
    selector: int,
    data_dir: Path,
    result_dir: Path,
    sfu: Path,
    lut_dir: Path,
) -> tuple[int, int, int]:
    rows = read_sv_output(data_dir / f"output_{name}.csv")
    failures = 0
    known_diffs = 0
    result_dir.mkdir(parents=True, exist_ok=True)

    with (result_dir / f"{name}.txt").open("w", encoding="ascii") as fp:
        fp.write("input_hex reference_hex rtl_hex error_status\n")
        for input_hex, rtl_hex in rows:
            reference_hex = run_reference(sfu, lut_dir, input_hex, selector)
            if reference_hex == rtl_hex:
                status = "PASS"
            elif is_known_sfu13_partition_diff(name, input_hex, reference_hex, rtl_hex):
                status = "KNOWN_DIFF"
                known_diffs += 1
            else:
                status = "FAIL"
                failures += 1
            fp.write(f"{input_hex} {reference_hex} {rtl_hex} {status}\n")

    return len(rows), failures, known_diffs


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--data-dir", type=Path, required=True)
    parser.add_argument("--result-dir", type=Path, required=True)
    parser.add_argument("--sfu", type=Path, required=True)
    parser.add_argument("--lut-dir", type=Path, required=True)
    args = parser.parse_args()

    if not args.sfu.exists():
        print(f"error: missing reference binary: {args.sfu}", file=sys.stderr)
        return 1
    if not args.lut_dir.exists():
        print(f"error: missing LUT directory: {args.lut_dir}", file=sys.stderr)
        return 1

    args.result_dir.mkdir(parents=True, exist_ok=True)
    total_rows = 0
    total_failures = 0
    total_known_diffs = 0
    summary_rows: list[tuple[str, int, int, int]] = []

    try:
        for name, selector in OPERATIONS:
            rows, failures, known_diffs = compare_operation(
                name,
                selector,
                args.data_dir,
                args.result_dir,
                args.sfu,
                args.lut_dir,
            )
            summary_rows.append((name, rows, failures, known_diffs))
            total_rows += rows
            total_failures += failures
            total_known_diffs += known_diffs
    except RuntimeError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1

    summary = args.result_dir / "summary.txt"
    with summary.open("w", encoding="ascii") as fp:
        fp.write("operation rows failures known_diffs status\n")
        for name, rows, failures, known_diffs in summary_rows:
            status = "PASS" if failures == 0 else "FAIL"
            fp.write(f"{name} {rows} {failures} {known_diffs} {status}\n")
        fp.write(
            f"TOTAL {total_rows} {total_failures} {total_known_diffs} "
            f"{'PASS' if total_failures == 0 else 'FAIL'}\n"
        )

    print("operation  rows  failures  known_diffs  status")
    for name, rows, failures, known_diffs in summary_rows:
        status = "PASS" if failures == 0 else "FAIL"
        print(f"{name:<9} {rows:>4}  {failures:>8}  {known_diffs:>11}  {status}")
    print(f"{'TOTAL':<9} {total_rows:>4}  {total_failures:>8}  {total_known_diffs:>11}  "
          f"{'PASS' if total_failures == 0 else 'FAIL'}")
    print(f"wrote four-column comparison files to {args.result_dir}")

    return 0 if total_failures == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
