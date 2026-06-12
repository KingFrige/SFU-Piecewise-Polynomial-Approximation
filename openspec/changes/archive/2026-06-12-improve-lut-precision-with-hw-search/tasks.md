## 1. Variant Plumbing

- [x] 1.1 Add `pdf-hw-search` to the variant enum, parser, CLI usage, tests, and metadata handling.
- [x] 1.2 Include `pdf-hw-search` in Makefile generation and comparison targets.

## 2. Hardware-Scaled Search

- [x] 2.1 Add fixed-point coefficient conversion and hardware-scaled segment error evaluation helpers.
- [x] 2.2 Implement bounded neighborhood search around the PDF 3-pass coefficients.
- [x] 2.3 Add tests that show `pdf-hw-search` is available and does not regress hardware-scaled error versus `pdf-quant` on representative segments.

## 3. Comparison and MPFR

- [x] 3.1 Extend `compare_variants` to report hardware-scaled error metrics.
- [x] 3.2 Use MPFR `sin`/`cos` in the optional MPFR target path.
- [x] 3.3 Add a compare summary script and wire it into `make compare` / `make compare-variants`.

## 4. Documentation and Verification

- [x] 4.1 Update `lut/README.md` with the paper-derived `pdf-hw-search` flow and accuracy interpretation.
- [x] 4.2 Run `make -C lut test`, `make -C lut compare`, and `openspec validate improve-lut-precision-with-hw-search --strict`.
