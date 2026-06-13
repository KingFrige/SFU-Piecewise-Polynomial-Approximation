#!/usr/bin/env python3
"""Compare SV results against SFU_13 VHDL output files."""

from __future__ import annotations

import argparse
import sys
from pathlib import Path


OPERATIONS = ["sin", "cos", "rsqrt", "log2", "ex2", "rcp", "sqrt"]


def read_output(path: Path) -> list[tuple[str, str]]:
    if not path.exists():
        raise RuntimeError(f"missing output file: {path}")

    rows: list[tuple[str, str]] = []
    with path.open("r", encoding="ascii") as fp:
        if next(fp, None) is None:
            raise RuntimeError(f"empty output file: {path}")
        for lineno, line in enumerate(fp, start=2):
            parts = [part.strip().upper() for part in line.split(",")]
            if len(parts) < 2 or len(parts[0]) != 8 or len(parts[1]) != 8:
                raise RuntimeError(f"{path}:{lineno}: malformed result row")
            rows.append((parts[0], parts[1]))
    return rows


def compare_operation(
    operation: str,
    sv_dir: Path,
    vhdl_dir: Path,
    result_dir: Path,
) -> tuple[int, int]:
    sv_rows = read_output(sv_dir / f"output_{operation}.csv")
    vhdl_rows = read_output(vhdl_dir / f"output_{operation}.csv")
    if len(sv_rows) != len(vhdl_rows):
        raise RuntimeError(
            f"{operation}: SV has {len(sv_rows)} rows, VHDL has {len(vhdl_rows)}"
        )

    failures = 0
    result_dir.mkdir(parents=True, exist_ok=True)
    with (result_dir / f"{operation}.txt").open("w", encoding="ascii") as fp:
        fp.write("input_hex vhdl_hex sv_hex error_status\n")
        for index, (sv_row, vhdl_row) in enumerate(zip(sv_rows, vhdl_rows), start=1):
            sv_input, sv_result = sv_row
            vhdl_input, vhdl_result = vhdl_row
            if sv_input != vhdl_input:
                raise RuntimeError(
                    f"{operation} row {index}: input order mismatch "
                    f"{sv_input} != {vhdl_input}"
                )
            status = "PASS" if sv_result == vhdl_result else "FAIL"
            if status == "FAIL":
                failures += 1
            fp.write(f"{sv_input} {vhdl_result} {sv_result} {status}\n")
    return len(sv_rows), failures


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--sv-dir", type=Path, required=True)
    parser.add_argument("--vhdl-dir", type=Path, required=True)
    parser.add_argument("--result-dir", type=Path, required=True)
    args = parser.parse_args()

    total_rows = 0
    total_failures = 0
    summary: list[tuple[str, int, int]] = []
    try:
        for operation in OPERATIONS:
            rows, failures = compare_operation(
                operation,
                args.sv_dir,
                args.vhdl_dir,
                args.result_dir,
            )
            summary.append((operation, rows, failures))
            total_rows += rows
            total_failures += failures
    except RuntimeError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1

    with (args.result_dir / "summary.txt").open("w", encoding="ascii") as fp:
        fp.write("operation rows failures status\n")
        for operation, rows, failures in summary:
            fp.write(
                f"{operation} {rows} {failures} "
                f"{'PASS' if failures == 0 else 'FAIL'}\n"
            )
        fp.write(
            f"TOTAL {total_rows} {total_failures} "
            f"{'PASS' if total_failures == 0 else 'FAIL'}\n"
        )

    print("operation  rows  failures  status")
    for operation, rows, failures in summary:
        print(
            f"{operation:<9} {rows:>4}  {failures:>8}  "
            f"{'PASS' if failures == 0 else 'FAIL'}"
        )
    print(
        f"{'TOTAL':<9} {total_rows:>4}  {total_failures:>8}  "
        f"{'PASS' if total_failures == 0 else 'FAIL'}"
    )
    print(f"wrote four-column comparison files to {args.result_dir}")
    return 0 if total_failures == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
