## 1. Structure and Build

- [x] 1.1 Create `cmodel/src`, `cmodel/tests`, and a `cmodel/Makefile`.
- [x] 1.2 Add public cmodel headers and a CLI entry point.

## 2. LUT Loading

- [x] 2.1 Implement LUT directory discovery and metadata parsing for generated LUT layouts.
- [x] 2.2 Implement binary row parsing for C0/C1/C2 widths and selector lookup.
- [x] 2.3 Add negative tests for missing or malformed LUT files.

## 3. SFU Evaluation

- [x] 3.1 Implement IEEE-754 bit helpers and selector mapping.
- [x] 3.2 Implement range reduction and function remap based on `Description/SFU_13` and `Octave/sfu_golden_model.m`.
- [x] 3.3 Implement fixed-point quadratic interpolation from loaded LUT rows.
- [x] 3.4 Implement output scaling, sign/exponent correction, and special-case handling.

## 4. Tests and Data

- [x] 4.1 Add deterministic C regression tests matching `Octave/test_sfu_golden_model.m`.
- [x] 4.2 Add txt data generation and comparison scripts under `cmodel/tests`.
- [x] 4.3 Add Makefile targets for `test`, `generate-data`, and `compare-octave`.
- [x] 4.4 Generate input-only txt files under `cmodel/build/input_data` with a configurable count that defaults to 16 values per function.
- [x] 4.5 Write Octave comparison results under `cmodel/build` with four columns: input data, golden result, cmodel result, and error status.

## 5. Documentation and Verification

- [x] 5.1 Document cmodel usage and the `Description/SFU_13` hardware reference.
- [x] 5.2 Run `make -C cmodel test`, generated-data comparison, and `openspec validate add-sfu-cmodel --strict`.
