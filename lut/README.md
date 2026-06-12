# LUT Coefficient Generation

This directory contains a standalone C implementation of the polynomial coefficient generation flow described in the project PDF section 5.2.

The generator produces the decimal `C0d`, `C1d`, and `C2d` coefficients for:

```text
f(x) ~= C0 + C1*x_l + C2*x_l^2
```

Each table is split into `2^m` uniform segments. The local variable `x_l` starts at zero inside each segment, matching the reference LUT addressing. The C generator also performs the fixed-point conversion and common-bit LUT compression needed for the final hardware bit-table format.

## Build

```sh
cd lut
make
```

## Generate Coefficients

Generate every coefficient directory:

```sh
./build/coeffgen build/remez-compat
```

The default C variant is `remez-compat`: Remez-generated coefficients plus the known compatibility corrections needed to match the Octave reference bit-for-bit. Generate exploratory variants with:

```sh
./build/coeffgen --variant=remez build/remez
./build/coeffgen --variant=pdf-quant build/pdf-quant
```

Generate one function by selector or name:

```sh
./build/coeffgen build/remez-compat exp
./build/coeffgen build/remez-compat 9
```

The output directory mirrors the Octave LUT layout: one directory per SFU LUT function, prefixed by selector number.

```text
build/remez-compat/01_reci/
build/remez-compat/06_exp/
build/remez-compat/09_sin/
```

Each function directory contains independent coefficient files:

```text
LUTC0.txt
LUTC1.txt
LUTC2.txt
metadata.txt
```

`LUTC0.txt`, `LUTC1.txt`, and `LUTC2.txt` contain fixed-width binary rows: 29 bits for C0, 20 bits for C1, and 14 bits for C2. `metadata.txt` records `selector`, `name`, `m`, `rows`, and the LUT row widths.

## Test

```sh
cd lut
make test
```

The test is C-only. It checks the PDF's `2^x` first-segment minimax example, validates all function metadata, generates all 896 coefficient rows, and verifies the emitted LUT files are binary rows with the expected widths. Use the `Octave/` Makefile separately when you need Octave-generated reference LUT output.

Run an explicit comparison against Octave-generated LUT files:

```sh
make compare-octave
```

This target invokes `Octave/Makefile generate-luts`, then compares directory layout, metadata, row widths, and binary row values. The comparison is bit-exact for `LUTC0.txt`, `LUTC1.txt`, and `LUTC2.txt`.

Generate and compare all supported C variants:

```sh
make compare-variants
```

This writes `build/remez-compat`, `build/remez`, and `build/pdf-quant`. If MPFR/GMP development libraries are available, it also builds `build/coeffgen_mpfr` and writes `build/mpfr-remez`.

Run the complete LUT generation and comparison flow:

```sh
make compare
```

This generates `build/octave_generated_luts` plus every supported C variant, checks `build/remez-compat` against the Octave reference, then prints the variant mismatch and approximation-error report.

## Accuracy and Compatibility

The C generator does not depend on Maple or Mathematica. It recomputes degree-2 minimax coefficients with a local Remez solver, then emits the same compressed binary LUT format as the Octave reference flow.

Available variants:

- `remez-compat`: default output. Uses the C Remez path plus the known Octave-reference C0 boundary-row compatibility adjustments so `make compare-octave` remains bit-exact.
- `remez`: raw long-double Remez output with no reference-compatibility adjustments.
- `pdf-quant`: applies PDF 5.2.2 compensated quantization: round `C1`, compensate/round `C2`, recompute/round `C0`.
- `mpfr-remez`: optional MPFR/GMP build of the Remez solve. Build with `make mpfr` and run `./build/coeffgen_mpfr --variant=mpfr-remez <out>`.

The PDF section 5.2 describes the intended method:

- solve a degree-2 minimax approximation per uniform segment;
- quantize `C1`, compensate `C2`, then recompute/quantize `C0`;
- store compressed coefficient tables with the bus widths summarized in PDF tables 4 and 5.

The repository reference source is `Golden-model/coeff.m`, which contains static decimal coefficients rather than the original Maple/Mathematica script. Most C-recomputed coefficients encode to the same fixed-point rows directly. A small set of `C0` values lies exactly on truncation boundaries after conversion to the current hardware bit format, so the `remez-compat` variant applies documented one-LSB compatibility adjustments for those rows before writing files. This keeps generated LUTs bit-exact with the checked-in Octave behavior while keeping the default generation path independent of Maple and Octave.

Further precision work should be evaluated against two separate goals:

- Reference compatibility: keep `make compare-octave` at zero mismatches.
- Mathematical approximation quality: experiment with the PDF 5.2.2 compensated quantization or an MPFR-backed Remez path, then measure end-to-end SFU error before replacing the compatibility table.

Current `make compare-variants` results on this environment:

```text
variant=remez-compat fixed_error max_abs=5.8738995011e-07 rms=8.9929021242e-08
variant=remez        fixed_error max_abs=5.8738995011e-07 rms=8.9958407443e-08
variant=pdf-quant    fixed_error max_abs=9.3931527781e-08 rms=1.5263449434e-08
variant=mpfr-remez   fixed_error max_abs=5.8738995011e-07 rms=8.9958407443e-08
```

`source_error` in the report measures the unencoded coefficient polynomial. `fixed_error` measures coefficients after the LUT fixed-point truncation used by the current writer. In this sample, `remez`/`mpfr-remez` slightly improve unencoded RMS error, but the reference-compatible fixed-point table remains marginally better after encoding. `pdf-quant` has lower max fixed-point error but a higher RMS error and many bit differences, so it is an exploration output, not a replacement for the Octave reference.

## Implementation Notes

- The polynomial degree is fixed at two.
- Initial Remez points are `0`, `h/3`, `2h/3`, and `h`, where `h = 2^-m`.
- Each iteration solves the four-equation alternation system and updates points from the extrema of `f(x) - p(x)`.
- `ln2e0` generates `log2(1+x)/x`, using the analytic limit at `x = 0`.
- `cos` segment 0 is emitted as the current `coeff.m` compatibility value `1, 0, -0.4998779296875`; all other segments use the generic Remez path.
- `coeffgen_generate_segment_variant(..., COEFFGEN_VARIANT_REMEZ_COMPAT, ...)` contains the Octave-reference C0 boundary-row compatibility adjustments.
