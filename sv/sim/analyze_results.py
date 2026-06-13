#!/usr/bin/env python3
"""Analyze SV Verilator SFU output CSV files against NumPy references."""

from __future__ import annotations

import argparse
import struct
import sys
from pathlib import Path

import numpy as np


OPERATIONS = ["sin", "cos", "rsqrt", "log2", "ex2", "rcp", "sqrt"]
SPECIAL_ROW_COUNT = 8


def hex_to_float(value: str) -> np.float32:
    return np.float32(struct.unpack(">f", struct.pack(">I", int(value, 16)))[0])


def read_output(
    path: Path,
    expected_data_rows: int,
) -> tuple[np.ndarray, np.ndarray]:
    rows: list[tuple[np.float32, np.float32]] = []
    expected_total_rows = SPECIAL_ROW_COUNT + expected_data_rows

    if not path.exists():
        raise RuntimeError(f"missing output file: {path}")

    with path.open("r", encoding="ascii") as fp:
        header = next(fp, None)
        if header is None:
            raise RuntimeError(f"empty output file: {path}")

        for lineno, line in enumerate(fp, start=2):
            parts = [part.strip() for part in line.split(",")]
            if len(parts) < 2 or not parts[0] or not parts[1]:
                raise RuntimeError(f"malformed row in {path}:{lineno}")
            rows.append((hex_to_float(parts[0]), hex_to_float(parts[1])))

    if len(rows) != expected_total_rows:
        raise RuntimeError(
            f"incomplete output file {path}: expected {expected_total_rows} result rows "
            f"({SPECIAL_ROW_COUNT} special + {expected_data_rows} generated), "
            f"got {len(rows)}"
        )

    data_rows = rows[SPECIAL_ROW_COUNT:]
    return np.float32([row[0] for row in data_rows]), np.float32([row[1] for row in data_rows])


def reference(operation: str, inputs: np.ndarray) -> np.ndarray:
    with np.errstate(all="ignore"):
        if operation == "sin":
            return np.float32(np.sin(inputs))
        if operation == "cos":
            return np.float32(np.cos(inputs))
        if operation == "rsqrt":
            return np.float32(1 / np.sqrt(inputs))
        if operation == "log2":
            return np.float32(np.log2(inputs))
        if operation == "ex2":
            return np.float32(2 ** inputs)
        if operation == "rcp":
            return np.float32(1 / inputs)
        if operation == "sqrt":
            return np.float32(np.sqrt(inputs))
    raise ValueError(f"unsupported operation: {operation}")


def format_line(operation: str, real: np.ndarray, actual: np.ndarray) -> str:
    abs_err = np.abs(np.float32(real) - np.float32(actual))
    denominator = np.where(real == 0, real + np.float32(1.0), real)
    numerator = np.where(real == 0, (real + np.float32(1.0)) - actual, real - actual)
    rel_err = np.abs(numerator / denominator)
    return (
        f"{operation} abs mean error=   {np.mean(abs_err)}"
        f"   Std={np.std(abs_err)}"
        f"  {operation} rel mean error=   {np.mean(rel_err)}"
        f"   Std={np.std(rel_err)}"
    )


def analyze(data_dir: Path, expected_data_rows: int) -> list[str]:
    lines = []
    for operation in OPERATIONS:
        inputs, actual = read_output(
            data_dir / f"output_{operation}.csv",
            expected_data_rows,
        )
        lines.append(format_line(operation, reference(operation, inputs), actual))
    return lines


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--data-dir", type=Path, default=Path("build/SFU_Input_data"))
    parser.add_argument("--n-data", type=int, default=16)
    args = parser.parse_args()

    try:
        lines = analyze(args.data_dir, args.n_data)
    except RuntimeError as exc:
        print(f"error: {exc}", file=sys.stderr)
        return 1

    output_path = args.data_dir / "Error_analysis.txt"
    with output_path.open("w", encoding="ascii") as fp:
        for line in lines:
            fp.write(f"{line}\n")

    for line in lines:
        print(line)
    print(f"wrote {output_path}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
