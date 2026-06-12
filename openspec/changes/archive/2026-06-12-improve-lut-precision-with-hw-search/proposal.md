## Why

The current C LUT generator implements the PDF 3-pass coefficient quantization only as a simple `pdf-quant` variant and evaluates accuracy with sparse mathematical samples. The QuadraticIEEETC0305 paper relies on hardware-behavior simulation and finite-wordlength effects, so the C flow needs a closer evaluation/search path before any higher-precision LUT can be considered.

## What Changes

- Add a `pdf-hw-search` C generation variant that starts from the PDF 3-pass quantized coefficients and searches nearby fixed-point coefficient values using hardware-scaled error.
- Extend the variant comparison report with a hardware-scaled fixed-point error metric in addition to existing source/fixed polynomial metrics.
- Include `pdf-hw-search` in `make generate-variants`, `make compare-variants`, and `make compare`.
- Improve optional MPFR-backed generation so trigonometric target evaluation uses MPFR functions where available.
- Document the new variant, its relationship to the paper, and how to interpret the accuracy report.

## Capabilities

### New Capabilities

### Modified Capabilities
- `lut-generation-variants`: Add a hardware-search variant and hardware-scaled accuracy reporting for LUT generation experiments.

## Impact

- Affects `lut/coeffgen.*`, `lut/coeffgen_mpfr.c`, `lut/compare_variants.c`, `lut/Makefile`, `lut/test_coeffgen.c`, and `lut/README.md`.
- No change to default `remez-compat` behavior or Octave bit-exact comparison.
- No new mandatory dependencies; MPFR/GMP remain optional.
