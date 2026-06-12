## MODIFIED Requirements

### Requirement: Selectable LUT Generation Variants
The LUT generator SHALL support multiple coefficient-generation variants without changing the default Octave-reference-compatible behavior.

#### Scenario: Default variant remains Octave-reference-compatible
- **WHEN** the user runs `./build/coeffgen <output_dir>` without a variant option
- **THEN** the generator writes LUT files that match the Octave reference bit-for-bit

#### Scenario: Explicit PDF quantization generation
- **WHEN** the user runs `./build/coeffgen --variant=pdf-quant <output_dir>`
- **THEN** the generator writes LUT files using the PDF 3-pass compensated quantization flow

#### Scenario: Explicit hardware-search generation
- **WHEN** the user runs `./build/coeffgen --variant=pdf-hw-search <output_dir>`
- **THEN** the generator writes LUT files using PDF 3-pass quantization plus fixed-point hardware-scaled coefficient search

#### Scenario: Unsupported variant is rejected
- **WHEN** the user passes an unknown `--variant` value
- **THEN** the generator exits with a non-zero status and prints the supported variant names

### Requirement: Variant Accuracy Comparison
The repository SHALL provide a task that generates all supported variants and compares them against the default compatibility reference.

#### Scenario: Compare all variants
- **WHEN** the user runs the variant comparison task
- **THEN** the task reports per-variant LUT bit mismatch counts against the default compatibility reference and approximation-error metrics
- **AND** the task reports hardware-scaled fixed-point approximation-error metrics
- **AND** the task appends a compact summary table and recommendation based on compatibility and error metrics
- **AND** the summary table includes a `golden-octave` reference row

#### Scenario: Full LUT comparison task
- **WHEN** the user runs `make compare` in `lut/`
- **THEN** the task generates Octave reference LUTs and all supported C variant LUTs
- **AND** it compares `remez-compat` against the Octave reference
- **AND** it reports per-variant LUT bit mismatch counts, polynomial approximation-error metrics, and hardware-scaled error metrics
- **AND** it appends the same summary table and recommendation

#### Scenario: Compatibility comparison remains exact
- **WHEN** the comparison task evaluates the `remez-compat` variant
- **THEN** all generated `LUTC0.txt`, `LUTC1.txt`, and `LUTC2.txt` rows match the Octave reference
