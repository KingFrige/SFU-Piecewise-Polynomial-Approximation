## 1. Variant Plumbing

- [x] 1.1 Add generator variant enums, CLI parsing, and metadata output for `remez-compat`, `remez`, `pdf-quant`, and optional `mpfr-remez`.
- [x] 1.2 Preserve default `remez-compat` output and the existing bit-exact `compare-octave` behavior.

## 2. Accuracy Variants

- [x] 2.1 Implement PDF 5.2.2 compensated quantization for fixed-point coefficient exploration.
- [x] 2.2 Add optional MPFR-backed Remez generation when MPFR/GMP are available.

## 3. Comparison Tooling

- [x] 3.1 Add a variant comparison/report tool for bit mismatch and approximation-error metrics.
- [x] 3.2 Add Makefile tasks to generate all supported variants and run the comparison report.

## 4. Tests and Documentation

- [x] 4.1 Extend C tests to cover variant selection and PDF quantization sanity checks.
- [x] 4.2 Update `lut/README.md` with variant commands, dependencies, and accuracy interpretation.
- [x] 4.3 Run default tests, Octave reference comparison, and variant comparison.

## 5. Output Layout Refinement

- [x] 5.1 Remove the `build/variants/` nesting and write variant outputs directly under `build/<variant>/`.
- [x] 5.2 Make `make test` generate the default C output under `build/remez-compat/` instead of `build/generated_coeffs/`.
- [x] 5.3 Re-run default tests, Octave reference comparison, and variant comparison after the output layout change.

## 6. Variant Naming Refinement

- [x] 6.1 Rename the C compatibility variant from `golden` to `remez-compat` to reserve Golden for the Octave/Golden-model reference.
- [x] 6.2 Update Makefile paths, comparison labels, tests, and documentation to use `build/remez-compat/`.

## 7. Unified Compare Task

- [x] 7.1 Add `make compare` to generate Octave reference LUTs and all supported C variant LUTs.
- [x] 7.2 Have `make compare` run the Octave bit-exact comparison and the variant mismatch/error report.
