## Why

The current C LUT generator matches the Octave reference tables, but it exposes only one effective output mode. We need multiple coefficient-generation variants to explore higher approximation accuracy without losing the existing reference-compatible baseline.

## What Changes

- Add selectable LUT generation variants:
  - `remez-compat` output for current bit-exact behavior.
  - PDF 5.2.2 compensated quantization output.
  - MPFR-backed high-precision Remez output when MPFR/GMP are available.
- Add comparison tooling that evaluates each variant against the Octave reference and reports bit-level and approximation-error differences.
- Keep the default `lut/` build and test path independent of Maple and Octave.
- Document how to generate, compare, and interpret each variant.

## Capabilities

### New Capabilities
- `lut-generation-variants`: Generate and compare multiple LUT coefficient variants for accuracy exploration.

### Modified Capabilities

None.

## Impact

- Affects `lut/` C generator, tests, Makefile targets, and README documentation.
- May add optional MPFR/GMP build targets; the default build must still work with only the C standard library and `libm`.
- Adds generated-output comparison tasks but should not commit generated LUT tables.
