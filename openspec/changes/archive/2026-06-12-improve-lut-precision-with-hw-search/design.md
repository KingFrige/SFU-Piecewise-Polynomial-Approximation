## Context

`lut/` currently supports `remez-compat`, `remez`, `pdf-quant`, and optional `mpfr-remez`. The `pdf-quant` variant implements the paper's 3-pass idea at coefficient level: round `C1`, compensate/round `C2`, then recompute/round `C0`. The comparison tool reports LUT bit differences and sparse polynomial error, but the paper's coefficient selection is driven by finite-wordlength hardware behavior and exhaustive simulation.

## Goals / Non-Goals

**Goals:**
- Add a new exploratory variant that improves hardware-scaled fixed-point error without changing the default compatibility output.
- Evaluate candidate coefficients with the same scaled integer datapath model used by the current Golden/Octave flow.
- Keep the implementation deterministic and C-only for the default build.
- Keep MPFR optional, but make its trigonometric target path genuinely MPFR-backed.

**Non-Goals:**
- Do not replace `remez-compat` as the default generator output.
- Do not implement a complete exhaustive IEEE-754 SFU validation flow in this change.
- Do not change VHDL LUT contents.

## Decisions

- Add `pdf-hw-search` as a separate variant. This keeps the paper-inspired hardware search visible without conflating it with `pdf-quant` or the Octave-compatible output.
- Search fixed-point integer neighborhoods around the PDF 3-pass result. For each segment, try small LSB deltas for encoded `C1` and `C2`; for each pair, compute the best continuous `C0`, then test nearby fixed-point `C0` integers.
- Rank candidates by maximum hardware-scaled error first, then RMS hardware-scaled error. This follows the paper's priority of bounded final error while providing a stable tie-breaker.
- Reuse existing target functions and fixed-point bit widths. Hardware-scaled error will model the LUT polynomial accumulation scale rather than full IEEE exponent/range reconstruction.
- Update `compare_variants` to report the same hardware-scaled metric so `pdf-hw-search` improvements are visible in normal `make compare`.

## Risks / Trade-offs

- Search cost grows quickly with neighborhood size. Mitigation: use a bounded small neighborhood and a fixed sample grid suitable for CI-scale tests.
- Hardware-scaled segment error is still not a full end-to-end SFU ULP proof. Mitigation: document it as a LUT-level metric and keep Octave compatibility separate.
- A variant can improve hardware-scaled metrics while increasing bit differences from the reference. Mitigation: report both bit mismatches and error metrics.
