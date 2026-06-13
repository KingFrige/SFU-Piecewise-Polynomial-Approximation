## ADDED Requirements

### Requirement: GHDL Verification Entry Point
The repository SHALL provide a `Description/SFU_13/ghdl/` GNU Make flow that can generate input data, compile SFU_13 RTL with GHDL, run `sfu_tb`, and analyze results.

#### Scenario: Default make flow
- **WHEN** a contributor runs `make` from `Description/SFU_13/ghdl/`
- **THEN** the flow generates `SFU_Input_data/input_*.csv`
- **AND** it analyzes and elaborates the SFU_13 VHDL sources with GHDL
- **AND** it runs `sfu_tb`
- **AND** it writes `SFU_Input_data/Error_analysis.txt`

#### Scenario: Clean generated outputs
- **WHEN** a contributor runs `make clean` from `Description/SFU_13/ghdl/`
- **THEN** GHDL work files and generated `SFU_Input_data/` outputs are removed

### Requirement: Linux-Compatible Input Data Generation
The GHDL flow SHALL generate the SFU input CSV files without relying on Windows shell commands.

#### Scenario: Input data layout
- **WHEN** the GHDL data generation target runs
- **THEN** it creates `SFU_Input_data/input_sin.csv`, `input_cos.csv`, `input_rsqrt.csv`, `input_log2.csv`, `input_ex2.csv`, `input_rcp.csv`, and `input_sqrt.csv`
- **AND** each file begins with the `celldata` header expected by `sfu_tb.vhd`
- **AND** each file includes the inherited special IEEE-754 edge-case hex values

#### Scenario: Configurable sample count
- **WHEN** a contributor runs the data generation target with an `N_DATA` override
- **THEN** each operation receives the requested number of random data rows in addition to the fixed special-case rows

### Requirement: GHDL Source Compilation
The GHDL flow SHALL compile the SFU_13 VHDL source set in a GHDL-compatible order without including backup files.

#### Scenario: Source analysis
- **WHEN** the GHDL analysis target runs
- **THEN** it uses VHDL-93-compatible GHDL flags with Synopsys textio support
- **AND** it analyzes `voter.vhd` before `Quadratic_Interpolator.vhd`
- **AND** it analyzes `Description/SFU_13/SFU/RRO/rro.vhd` using the actual Linux filename case

### Requirement: Result Analysis Compatibility
The GHDL flow SHALL preserve the existing verification result semantics for all supported SFU operations.

#### Scenario: Error analysis report
- **WHEN** the GHDL simulation completes successfully
- **THEN** the result analysis reads the generated `output_*.csv` files
- **AND** it computes absolute and relative error statistics for `sin`, `cos`, `rsqrt`, `log2`, `ex2`, `rcp`, and `sqrt`
- **AND** it writes the statistics to `SFU_Input_data/Error_analysis.txt`

### Requirement: GHDL Flow Documentation
The repository SHALL document how to run the SFU_13 GHDL flow and interpret its generated files.

#### Scenario: Contributor reads GHDL README
- **WHEN** a contributor opens `Description/SFU_13/ghdl/README.md`
- **THEN** the document lists required tools, main Make targets, generated output paths, and known GHDL/ModelSim differences
