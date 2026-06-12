## Context

`lut/` currently has a standalone C Remez generator and a reference-compatible LUT writer. The Octave reference path is based on static decimal coefficients in `Golden-model/coeff.m`; it is useful as the compatibility baseline, but it is not enough to explore whether PDF 5.2.2 compensated quantization or higher-precision arithmetic can improve approximation error.

## Goals / Non-Goals

**Goals:**

- Preserve the current default Octave-reference-compatible output and bit-exact Octave comparison.
- Add selectable coefficient variants for baseline Remez, PDF 5.2.2 compensated quantization, and MPFR-backed Remez.
- Generate separate output directories for each variant so tables can be inspected and consumed independently.
- Add a comparison task that reports bit mismatch counts against the default compatibility reference and approximation-error metrics across variants.

**Non-Goals:**

- Do not replace the checked-in Octave reference tables as the default truth source.
- Do not require Maple, Mathematica, MPFR, GMP, or Octave for the default C-only test.
- Do not modify VHDL LUT files in this change.

## Decisions

- Keep `remez-compat` as the default CLI mode. In the C generator this means the Remez path plus the known compatibility corrections needed to match the Octave reference.
- Add `--variant=<name>` to the C generator. Supported names are `remez-compat`, `remez`, `pdf-quant`, and `mpfr-remez` when MPFR support is compiled in.
- Implement PDF 5.2.2 compensation after an unquantized Remez solve: round `C1`, compensate and round `C2`, then recompute a degree-0 minimax offset for `C0` with fixed `C1/C2`.
- Implement MPFR as a separate optional binary or build configuration. If MPFR/GMP are unavailable, the default build still succeeds and the MPFR target reports that it is unavailable.
- Add a C comparison/report tool that can compare generated LUT directories to the default compatibility reference and compute approximation-error summaries using the same target functions.

## Risks / Trade-offs

- PDF 5.2.2 says "round" while the current reference bit-table encoder truncates decimal values. Mitigation: treat `pdf-quant` as an exploration variant and report both bit differences and approximation error before considering it for hardware use.
- MPFR can improve numerical stability but may not change final fixed-point LUT bits. Mitigation: keep MPFR optional and compare it explicitly against `remez`.
- Better polynomial approximation may not improve end-to-end SFU error if hardware truncation dominates. Mitigation: report LUT-level polynomial error first and leave full datapath error analysis as a separate follow-up.
