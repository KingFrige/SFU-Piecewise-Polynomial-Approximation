# sfu-cmodel Specification

## Purpose
Define the standalone C SFU model, LUT loading behavior, Octave golden comparison flow, and hardware reference documentation expected for software verification.
## Requirements
### Requirement: C SFU Model Evaluation
The repository SHALL provide a standalone C model that evaluates supported SFU functions from IEEE-754 single-precision hexadecimal inputs.

#### Scenario: Evaluate supported selectors
- **WHEN** the user runs the cmodel CLI with an input hex value, a supported selector, and a LUT directory
- **THEN** the CLI prints the resulting IEEE-754 hex value
- **AND** the supported selectors include `1`, `2`, `4`, `6`, `7`, `9`, and `10`

#### Scenario: Reject unsupported selector
- **WHEN** the user runs the cmodel CLI with an unsupported selector
- **THEN** the command exits with a non-zero status and reports the supported selectors

### Requirement: LUT File Loading
The C model SHALL load coefficient tables from the existing generated LUT directory layout.

#### Scenario: Load generated LUT layout
- **WHEN** the model is given a LUT root containing selector directories such as `01_reci` and `06_exp`
- **THEN** it loads `LUTC0.txt`, `LUTC1.txt`, `LUTC2.txt`, and `metadata.txt` for the functions required by the evaluated selector

#### Scenario: Missing LUT files fail clearly
- **WHEN** a required LUT file is missing or has an invalid row width
- **THEN** the model exits with a non-zero status and reports the invalid path or table

### Requirement: Octave Golden Comparison
The repository SHALL provide tests that compare C model results against the Octave golden model.

#### Scenario: Deterministic regression vectors
- **WHEN** the user runs the cmodel test target
- **THEN** it evaluates deterministic vectors equivalent to `Octave/test_sfu_golden_model.m`
- **AND** every C result matches the Octave expected hex result

#### Scenario: Generated txt data comparison
- **WHEN** the user runs the generated-data comparison target
- **THEN** the test creates txt input data under `cmodel/build/input_data`
- **AND** each function input file contains the configured number of input values, defaulting to 16
- **AND** the comparison target writes results under `cmodel/build`
- **AND** the comparison target regenerates input data before comparing
- **AND** each result row uses four columns: input data, golden result, cmodel result, and error status

### Requirement: Hardware Reference Documentation
The cmodel documentation SHALL identify `Description/SFU_13` as the VHDL hardware reference.

#### Scenario: User inspects cmodel documentation
- **WHEN** the user reads the cmodel README or top-level build notes
- **THEN** the documentation states that the implementation is based on `Description/SFU_13` plus the Octave golden model acceptance oracle

### Requirement: Hardware-Aligned C Source Organization
The C SFU model SHALL keep hardware datapath logic separated from peripheral file and parsing helpers.

#### Scenario: SFU top-level source stays hardware-focused
- **WHEN** a contributor inspects `cmodel/src/sfu.c` and `cmodel/src/sfu.h`
- **THEN** those files expose the top-level SFU evaluation API and hardware datapath orchestration
- **AND** they do not contain LUT text-file parsing or filesystem path construction logic

#### Scenario: Peripheral LUT loading is isolated
- **WHEN** the cmodel loads generated LUT text files from disk
- **THEN** file layout discovery, metadata parsing, binary row parsing, and memory ownership for loaded tables are implemented outside `sfu.c`

#### Scenario: Module names align with SFU hardware structure
- **WHEN** a contributor inspects `cmodel/src`
- **THEN** source modules use names that correspond to the paper and `Description/SFU_13/SFU` structure, including RRO, quadratic interpolation, ROM, reconstruction or normalization, and SFU exceptions

#### Scenario: Refactor preserves golden behavior
- **WHEN** the user runs deterministic tests and Octave generated-data comparison
- **THEN** the C model results remain bit-equivalent to the Octave golden model for the tested vectors

