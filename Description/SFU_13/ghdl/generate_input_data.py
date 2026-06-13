#!/usr/bin/env python3
"""Generate SFU_13 input CSV files for the GHDL testbench."""

from __future__ import annotations

import argparse
import shutil
import struct
from pathlib import Path

import numpy as np


SPECIAL_CONDITIONS = [
    "ffffffff",  # NaN
    "ff800000",  # -inf
    "80000001",  # -subnormal
    "80000000",  # -0
    "00000000",  # +0
    "00000001",  # +subnormal
    "7f800000",  # +inf
    "7fffffff",  # NaN
]


def float_to_hex(value: np.float32) -> str:
    return f"{struct.unpack('<I', struct.pack('<f', float(value)))[0]:08x}"


def float_array_to_hex(values: np.ndarray) -> list[str]:
    return [float_to_hex(np.float32(value)) for value in values]


def write_csv(path: Path, values: list[str]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="ascii") as fp:
        fp.write("celldata\n")
        for value in values:
            fp.write(f"{value}\n")


def generate(output_dir: Path, n_data: int, seed: int) -> None:
    rng = np.random.default_rng(seed)

    if output_dir.exists():
        shutil.rmtree(output_dir)
    output_dir.mkdir(parents=True)

    data_sets = {
        "sin": np.sort(np.float32(np.pi * rng.uniform(-47.0, 47.0, n_data))),
        "cos": np.sort(np.float32(np.pi * rng.uniform(-47.0, 47.0, n_data))),
        "rsqrt": np.sort(np.float32(rng.uniform(0.0, 47.0, n_data))),
        "log2": np.sort(np.float32(rng.uniform(0.0, 47.0, n_data))),
        "ex2": np.sort(np.float32(rng.uniform(-47.0, 47.0, n_data))),
        "rcp": np.sort(np.float32(rng.uniform(-47.0, 47.0, n_data))),
        "sqrt": np.sort(np.float32(rng.uniform(0.0, 47.0, n_data))),
    }

    for name, values in data_sets.items():
        rows = list(SPECIAL_CONDITIONS)
        rows.extend(float_array_to_hex(values))
        write_csv(output_dir / f"input_{name}.csv", rows)

    print(f"generated {n_data} random row(s) per operation in {output_dir}")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output-dir", type=Path, default=Path("SFU_Input_data"))
    parser.add_argument("--n-data", type=int, default=20000)
    parser.add_argument("--seed", type=int, default=0x5F013)
    args = parser.parse_args()

    if args.n_data < 0:
        raise SystemExit("--n-data must be non-negative")

    generate(args.output_dir, args.n_data, args.seed)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
