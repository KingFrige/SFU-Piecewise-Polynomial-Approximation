#!/usr/bin/env python3
"""Summarize compare_variants output as a compact terminal table."""

from __future__ import annotations

import argparse
import re
import subprocess
import sys
from dataclasses import dataclass, field
from typing import Dict, List, Optional


VARIANT_RE = re.compile(r"^variant=([^ ]+) path=(.*)$")
BIT_RE = re.compile(
    r"^\s+(LUTC[012]\.txt) rows=(\d+) mismatches=(\d+) max_unsigned_diff=(\d+)$"
)
ERROR_RE = re.compile(
    r"^\s+(source_error|fixed_error|hw_error)\s+samples=(\d+) max_abs=([0-9.+\-Ee]+) rms=([0-9.+\-Ee]+)$"
)


@dataclass
class BitStats:
    rows: int
    mismatches: int
    max_unsigned_diff: int


@dataclass
class ErrorStats:
    samples: int
    max_abs: float
    rms: float


@dataclass
class VariantStats:
    name: str
    path: str
    bits: Dict[str, BitStats] = field(default_factory=dict)
    errors: Dict[str, ErrorStats] = field(default_factory=dict)

    @property
    def total_mismatches(self) -> int:
        return sum(stats.mismatches for stats in self.bits.values())

    @property
    def max_unsigned_diff(self) -> int:
        if not self.bits:
            return 0
        return max(stats.max_unsigned_diff for stats in self.bits.values())


def parse_compare_output(text: str) -> List[VariantStats]:
    variants: List[VariantStats] = []
    current: Optional[VariantStats] = None

    for line in text.splitlines():
        match = VARIANT_RE.match(line)
        if match:
            current = VariantStats(name=match.group(1), path=match.group(2))
            variants.append(current)
            continue

        if current is None:
            continue

        match = BIT_RE.match(line)
        if match:
            current.bits[match.group(1)] = BitStats(
                rows=int(match.group(2)),
                mismatches=int(match.group(3)),
                max_unsigned_diff=int(match.group(4)),
            )
            continue

        match = ERROR_RE.match(line)
        if match:
            current.errors[match.group(1)] = ErrorStats(
                samples=int(match.group(2)),
                max_abs=float(match.group(3)),
                rms=float(match.group(4)),
            )

    return variants


def format_sci(value: Optional[float]) -> str:
    if value is None:
        return "n/a"
    return f"{value:.4e}"


def best_by_error(variants: List[VariantStats], key: str) -> Optional[VariantStats]:
    candidates = [variant for variant in variants if key in variant.errors]
    if not candidates:
        return None
    return min(candidates, key=lambda variant: variant.errors[key].max_abs)


def compatibility_text(variant: VariantStats) -> str:
    if variant.total_mismatches == 0:
        return "0 mismatches"
    return f"{variant.total_mismatches} mismatches, max diff {variant.max_unsigned_diff}"


def comment_for(
    variant: VariantStats,
    compatibility_winner: Optional[VariantStats],
    fixed_winner: Optional[VariantStats],
    hw_winner: Optional[VariantStats],
) -> str:
    if compatibility_winner is variant:
        return "Best compatibility; keep as default reference."
    if hw_winner is variant:
        return "Best hardware-scaled accuracy; validate end-to-end before replacing default."
    if fixed_winner is variant:
        return "Best fixed-point max error; not reference-compatible."
    if variant.name == "pdf-quant":
        return "PDF compensated quantization baseline."
    if variant.name == "mpfr-remez":
        return "MPFR Remez path; no encoded-LUT gain in this run."
    if variant.name == "remez":
        return "Raw Remez path; close to default but not bit-exact."
    return "Exploration variant."


def table_border(widths: List[int], left: str = "+", fill: str = "-", join: str = "+", right: str = "+") -> str:
    return left + join.join(fill * (width + 2) for width in widths) + right


def table_row(values: List[str], widths: List[int]) -> str:
    cells = [f" {value:<{width}} " for value, width in zip(values, widths)]
    return "|" + "|".join(cells) + "|"


def format_table(headers: List[str], rows: List[List[str]]) -> List[str]:
    widths = [
        max(len(headers[index]), *(len(row[index]) for row in rows))
        for index in range(len(headers))
    ]
    lines = [table_border(widths), table_row(headers, widths), table_border(widths)]
    lines.extend(table_row(row, widths) for row in rows)
    lines.append(table_border(widths))
    return lines


def build_summary(variants: List[VariantStats]) -> str:
    compatibility_winners = [variant for variant in variants if variant.total_mismatches == 0]
    compatibility_winner = compatibility_winners[0] if compatibility_winners else None
    fixed_winner = best_by_error(variants, "fixed_error")
    hw_winner = best_by_error(variants, "hw_error")

    headers = [
        "Variant",
        "Reference compatibility",
        "fixed_error max",
        "hw_error max",
        "Comment",
    ]
    rows = [
        [
            "golden-octave",
            "reference",
            "n/a",
            "n/a",
            "Golden-model/Octave LUT reference; bit-exact baseline.",
        ]
    ]

    for variant in variants:
        fixed = variant.errors.get("fixed_error")
        hw = variant.errors.get("hw_error")
        rows.append(
            [
                variant.name,
                compatibility_text(variant),
                format_sci(fixed.max_abs if fixed else None),
                format_sci(hw.max_abs if hw else None),
                comment_for(
                    variant,
                    compatibility_winner,
                    fixed_winner,
                    hw_winner,
                ),
            ]
        )

    lines: List[str] = []
    lines.append("")
    lines.append("LUT Variant Summary")
    lines.append("")
    lines.extend(format_table(headers, rows))

    lines.append("")
    lines.append("Conclusion")

    if compatibility_winner is not None:
        lines.append(
            f"  Compatibility winner: {compatibility_winner.name} "
            f"({compatibility_text(compatibility_winner)})."
        )
    else:
        lines.append("  Compatibility winner: none of the reported variants is bit-exact.")

    if fixed_winner is not None:
        fixed = fixed_winner.errors["fixed_error"]
        lines.append(
            f"  Best fixed_error max_abs: {fixed_winner.name} ({format_sci(fixed.max_abs)})."
        )

    if hw_winner is not None:
        hw = hw_winner.errors["hw_error"]
        lines.append(
            f"  Best hw_error max_abs: {hw_winner.name} ({format_sci(hw.max_abs)})."
        )

    if compatibility_winner is not None and hw_winner is not None:
        if compatibility_winner is hw_winner:
            lines.append(
                f"  Recommendation: keep {compatibility_winner.name}; it is both "
                "reference-compatible and best by hw_error in this run."
            )
        else:
            lines.append(
                f"  Recommendation: keep {compatibility_winner.name} as the "
                f"Golden/Octave-compatible default, and use {hw_winner.name} for "
                "precision exploration."
            )

    return "\n".join(lines) + "\n"


def read_compare_output(command: List[str]) -> tuple[int, str]:
    if command:
        completed = subprocess.run(
            command,
            stdout=subprocess.PIPE,
            stderr=subprocess.STDOUT,
            text=True,
            check=False,
        )
        return completed.returncode, completed.stdout

    return 0, sys.stdin.read()


def parse_args(argv: List[str]) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Run or read compare_variants output, then print a terminal summary table."
        )
    )
    parser.add_argument(
        "--summary-only",
        action="store_true",
        help="do not echo the raw compare_variants output",
    )
    parser.add_argument(
        "command",
        nargs=argparse.REMAINDER,
        help="optional command to run, usually after --",
    )
    args = parser.parse_args(argv)
    if args.command and args.command[0] == "--":
        args.command = args.command[1:]
    return args


def main(argv: List[str]) -> int:
    args = parse_args(argv)
    returncode, output = read_compare_output(args.command)

    if output and not args.summary_only:
        print(output, end="")

    if returncode != 0:
        return returncode

    variants = parse_compare_output(output)
    if not variants:
        print("compare_summary.py: no variant records found in compare output", file=sys.stderr)
        return 1

    print(build_summary(variants), end="")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
