## ADDED Requirements

### Requirement: Selectable LUT Generation Variants
The LUT generator SHALL support multiple coefficient-generation variants without changing the default Octave-reference-compatible behavior.

#### Scenario: Default variant remains Octave-reference-compatible
- **WHEN** the user runs `./build/coeffgen <output_dir>` without a variant option
- **THEN** the generator writes LUT files that match the Octave reference bit-for-bit

#### Scenario: Explicit variant generation
- **WHEN** the user runs `./build/coeffgen --variant=pdf-quant <output_dir>`
- **THEN** the generator writes LUT files using the PDF 5.2.2 compensated quantization flow

#### Scenario: Unsupported variant is rejected
- **WHEN** the user passes an unknown `--variant` value
- **THEN** the generator exits with a non-zero status and prints the supported variant names

### Requirement: Optional MPFR Variant
The build SHALL provide an MPFR-backed Remez variant when MPFR/GMP are available, while keeping the default build independent of MPFR/GMP.

#### Scenario: MPFR target available
- **WHEN** MPFR/GMP development libraries are available and the user builds the MPFR target
- **THEN** the generator supports `--variant=mpfr-remez`

#### Scenario: Default build without MPFR
- **WHEN** MPFR/GMP are unavailable
- **THEN** `make test` still builds and runs the C-only non-MPFR tests

### Requirement: Variant Accuracy Comparison
The repository SHALL provide a task that generates all supported variants and compares them against the default compatibility reference.

#### Scenario: Compare all variants
- **WHEN** the user runs the variant comparison task
- **THEN** the task reports per-variant LUT bit mismatch counts against the default compatibility reference and approximation-error metrics

#### Scenario: Full LUT comparison task
- **WHEN** the user runs `make compare` in `lut/`
- **THEN** the task generates Octave reference LUTs and all supported C variant LUTs
- **AND** it compares `remez-compat` against the Octave reference
- **AND** it reports per-variant LUT bit mismatch counts and approximation-error metrics

#### Scenario: Compatibility comparison remains exact
- **WHEN** the comparison task evaluates the `remez-compat` variant
- **THEN** all generated `LUTC0.txt`, `LUTC1.txt`, and `LUTC2.txt` rows match the Octave reference

### Requirement: Variant Documentation
The LUT documentation SHALL explain each variant, its dependencies, and how to interpret comparison results.

#### Scenario: User reads LUT README
- **WHEN** the user opens `lut/README.md`
- **THEN** the document lists the available variants, commands to generate and compare them, and notes that higher mathematical accuracy is not automatically a hardware replacement decision
