#!/usr/bin/env python3
"""Generate SFU input CSV files for the SV Verilator testbench."""

from __future__ import annotations

import argparse
import shutil
import struct
from pathlib import Path

import numpy as np


SPECIAL_CONDITIONS = [
    "ffffffff",
    "ff800000",
    "80000001",
    "80000000",
    "00000000",
    "00000001",
    "7f800000",
    "7fffffff",
]

DIRECTED_CONDITIONS = {
    "sin": [
        "3fc90fdb",  # pi/2
        "40490fdb",  # pi
        "4096cbe4",  # 3*pi/2
        "40c90fdb",  # 2*pi
    ],
    "cos": [
        "3fc90fdb",
        "40490fdb",
        "4096cbe4",
        "40c90fdb",
    ],
    "ex2": [
        "7f7fffff",  # largest finite positive
        "ff7fffff",  # largest finite negative
        "43060000",  # +134
        "c3060000",  # -134
    ],
}


def float_to_hex(value: np.float32) -> str:
    return f"{struct.unpack('<I', struct.pack('<f', float(value)))[0]:08x}"


def write_csv(path: Path, values: list[str]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="ascii") as fp:
        fp.write("celldata\n")
        for value in values:
            fp.write(f"{value}\n")


def build_test_rows(name: str, values: np.ndarray, n_data: int) -> list[str]:
    directed = DIRECTED_CONDITIONS.get(name, [])[:n_data]
    random_count = n_data - len(directed)
    random_rows = [
        float_to_hex(np.float32(value))
        for value in values[:random_count]
    ]
    return list(SPECIAL_CONDITIONS) + directed + random_rows


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
        write_csv(
            output_dir / f"input_{name}.csv",
            build_test_rows(name, values, n_data),
        )

    print(
        f"generated {n_data} test row(s) per operation "
        f"(directed first, random fill) in {output_dir}"
    )


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--output-dir", type=Path, default=Path("build/SFU_Input_data"))
    parser.add_argument("--n-data", type=int, default=16)
    parser.add_argument("--seed", type=int, default=0x5F013)
    args = parser.parse_args()

    if args.n_data < 0:
        raise SystemExit("--n-data must be non-negative")

    generate(args.output_dir, args.n_data, args.seed)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
