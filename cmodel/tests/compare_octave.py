#!/usr/bin/env python3
"""Generate SFU input data and compare the C model against Octave."""

from __future__ import annotations

import argparse
import random
import re
import struct
import subprocess
from pathlib import Path


FUNCTIONS = {
    "reci": (1, [1.0, 2.0, 3.0, -2.0]),
    "sqrt_1_2": (2, [1.0, 2.0, 4.0, 9.0]),
    "reci_sqrt_1_2": (4, [1.0, 2.0, 4.0, 9.0]),
    "exp": (6, [-0.5, 0.0, 0.5, 1.0]),
    # Avoid log2(1.0): the current Octave dec2hex754 helper crashes on exact zero.
    "ln2": (7, [1.5, 2.0, 3.0, 4.0]),
    "sin": (9, [0.5, 3.1415927 / 4.0, 2.0, -2.0]),
    "cos": (10, [0.5, 3.1415927 / 4.0, 2.0, -46.55347]),
}

RANGES = {
    "reci": (-47.0, 47.0),
    "sqrt_1_2": (0.001, 47.0),
    "reci_sqrt_1_2": (0.001, 47.0),
    "exp": (-8.0, 8.0),
    "ln2": (1.001, 47.0),
    "sin": (-47.0 * 3.1415927, 47.0 * 3.1415927),
    "cos": (-47.0 * 3.1415927, 47.0 * 3.1415927),
}

REPO_ROOT = Path(__file__).resolve().parents[2]
DEFAULT_COUNT = 16
BATCH_RESULT_RE = re.compile(r"^SFU_BATCH_RESULT\s+([0-9A-Fa-f]{8})\s+([0-9A-Fa-f]{8})$")


def float_to_hex32(value: float) -> str:
    return f"{struct.unpack('<I', struct.pack('<f', value))[0]:08X}"


def octave_string(value: Path | str) -> str:
    return "'" + str(value).replace("'", "''") + "'"


def run_octave_batch(octave: str, input_path: Path, selector: int) -> list[tuple[str, str]]:
    code = "\n".join(
        [
            f"fid = fopen({octave_string(input_path.resolve())}, 'r');",
            "if fid < 0, error('failed to open input file'); endif",
            "while true",
            "  input_hex = fgetl(fid);",
            "  if !ischar(input_hex), break; endif",
            "  input_hex = upper(strtrim(input_hex));",
            "  if isempty(input_hex) || strncmp(input_hex, '#', 1), continue; endif",
            f"  [result_hex, result_dec] = sfu_golden_model(input_hex, {selector});",
            "  printf('SFU_BATCH_RESULT %s %s\\n', input_hex, upper(result_hex));",
            "endwhile",
            "fclose(fid);",
        ]
    )
    completed = subprocess.run(
        [
            octave,
            "--no-gui",
            "--no-window-system",
            "--quiet",
            "--eval",
            code,
        ],
        cwd=REPO_ROOT / "Octave",
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        check=False,
    )
    if completed.returncode != 0:
        raise RuntimeError(completed.stdout)

    results = []
    for line in completed.stdout.splitlines():
        match = BATCH_RESULT_RE.match(line.strip())
        if match:
            results.append((match.group(1).upper(), match.group(2).upper()))
    return results


def run_sfu(sfu: Path, lut_dir: Path, input_hex: str, selector: int) -> str:
    completed = subprocess.run(
        [str(sfu), str(lut_dir), input_hex, str(selector)],
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        check=False,
    )
    if completed.returncode != 0:
        raise RuntimeError(completed.stdout)
    for line in completed.stdout.splitlines():
        if line.startswith("result_hex="):
            return line.split("=", 1)[1].strip().upper()
    raise RuntimeError(f"missing result_hex in SFU output:\n{completed.stdout}")


def generate_inputs(name: str, count: int) -> list[str]:
    selector, seeds = FUNCTIONS[name]
    del selector
    low, high = RANGES[name]
    rng = random.Random(0x5F000000 + sum(ord(ch) for ch in name))
    values = [float_to_hex32(value) for value in seeds]

    while len(values) < count:
        value = rng.uniform(low, high)
        if name in {"reci", "exp", "sin", "cos"}:
            pass
        elif value <= 0.0:
            continue
        values.append(float_to_hex32(value))

    return values[:count]


def write_input_file(path: Path, inputs: list[str]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="ascii") as fp:
        for input_hex in inputs:
            fp.write(f"{input_hex}\n")


def read_input_file(path: Path) -> list[str]:
    inputs = []
    with path.open("r", encoding="ascii") as fp:
        for lineno, line in enumerate(fp, start=1):
            stripped = line.strip()
            if not stripped or stripped.startswith("#"):
                continue
            if len(stripped) != 8:
                raise ValueError(f"{path}:{lineno}: expected 8 hex characters")
            inputs.append(stripped.upper())
    return inputs


def compare_one(
    name: str,
    inputs: list[str],
    sfu: Path,
    lut_dir: Path,
    output_dir: Path,
    data_dir: Path,
    octave: str,
) -> int:
    selector = FUNCTIONS[name][0]
    output_dir.mkdir(parents=True, exist_ok=True)
    octave_results = run_octave_batch(octave, data_dir / f"{name}.txt", selector)
    if len(octave_results) != len(inputs):
        raise RuntimeError(
            f"Octave returned {len(octave_results)} rows for {name}, expected {len(inputs)}"
        )
    failures = 0
    with (output_dir / f"{name}.txt").open("w", encoding="ascii") as fp:
        fp.write("input_hex golden_hex cmodel_hex error_status\n")
        for input_hex, (octave_input_hex, golden_hex) in zip(inputs, octave_results):
            if octave_input_hex != input_hex:
                raise RuntimeError(
                    f"Octave output order mismatch for {name}: {octave_input_hex} != {input_hex}"
                )
            cmodel_hex = run_sfu(sfu, lut_dir, input_hex, selector)
            error_status = "PASS" if golden_hex == cmodel_hex else "FAIL"
            if error_status != "PASS":
                failures += 1
            fp.write(f"{input_hex} {golden_hex} {cmodel_hex} {error_status}\n")
    return failures


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--sfu", required=True, type=Path)
    parser.add_argument("--lut-dir", required=True, type=Path)
    parser.add_argument("--data-dir", required=True, type=Path)
    parser.add_argument("--result-dir", required=True, type=Path)
    parser.add_argument("--count", type=int, default=DEFAULT_COUNT)
    parser.add_argument("--octave", default="octave")
    parser.add_argument("--generate-only", action="store_true")
    args = parser.parse_args()

    if args.count <= 0:
        raise SystemExit("--count must be positive")

    total = 0
    for name in FUNCTIONS:
        path = args.data_dir / f"{name}.txt"
        if args.generate_only or not path.exists():
            inputs = generate_inputs(name, args.count)
            write_input_file(path, inputs)
        else:
            inputs = read_input_file(path)
            if len(inputs) != args.count:
                inputs = generate_inputs(name, args.count)
                write_input_file(path, inputs)
        total += len(inputs)

    print(f"input data ready: {total} rows under {args.data_dir}")
    if args.generate_only:
        return 0

    failures = 0
    for name in FUNCTIONS:
        inputs = read_input_file(args.data_dir / f"{name}.txt")
        failures += compare_one(
            name,
            inputs,
            args.sfu,
            args.lut_dir,
            args.result_dir,
            args.data_dir,
            args.octave,
        )

    print(f"wrote four-column comparison results to {args.result_dir}")
    if failures:
        print(f"{failures} SFU/Octave mismatches")
        return 1
    print("all SFU/Octave generated-data comparisons passed")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
