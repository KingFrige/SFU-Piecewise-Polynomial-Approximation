## ADDED Requirements

### Requirement: SystemVerilog RTL Port
The repository SHALL provide a SystemVerilog/Verilog SFU RTL implementation under `sv/SFU` that follows the `Description/SFU_13/SFU` hardware structure.

#### Scenario: Hardware modules are present
- **WHEN** a contributor inspects `sv/SFU`
- **THEN** the directory contains RTL modules corresponding to RRO, SFU top-level, quadratic interpolation, ROM/LUT access, CLZ, reconstruction or normalization, SFU exceptions, squaring, fused accumulation, and voter behavior

#### Scenario: SFU_13 interface is preserved
- **WHEN** a contributor inspects the SV top-level SFU module
- **THEN** it exposes the SFU_13-style `src1_i`, `selop_i`, `Result_o`, and `Quad_int_err` interface

#### Scenario: RTL is synthesizable
- **WHEN** a contributor scans modules under `sv/SFU`
- **THEN** datapath computation uses fixed-width synthesizable logic
- **AND** it does not use SystemVerilog `real`, `shortreal`, or simulation-only math system functions

### Requirement: Verilator SV Testbench Flow
The repository SHALL provide a `sv/sim` Verilator simulation flow driven by a SystemVerilog testbench.

#### Scenario: Build and run with Make
- **WHEN** a contributor runs `make` from `sv/sim`
- **THEN** the flow builds the SV RTL with Verilator
- **AND** it runs the SV testbench
- **AND** it writes per-function output files under the simulation build area

#### Scenario: No C++ stimulus harness
- **WHEN** a contributor inspects `sv/sim`
- **THEN** stimulus sequencing is implemented in a SystemVerilog testbench
- **AND** the flow does not require a project-maintained C++ harness to drive the DUT

### Requirement: GHDL-Style Verification Coverage
The Verilator flow SHALL preserve the verification coverage shape from `Description/SFU_13/ghdl`.

#### Scenario: Generated input data
- **WHEN** the data generation target runs
- **THEN** it creates input files for `sin`, `cos`, `rsqrt`, `log2`, `ex2`, `rcp`, and `sqrt`
- **AND** each file includes inherited special IEEE-754 edge-case values
- **AND** each file includes exactly the configured number of generated test values
- **AND** directed boundary values consume that configured count before random values fill the remaining rows

#### Scenario: Result analysis
- **WHEN** the Verilator simulation completes
- **THEN** the analysis step reads per-function output files
- **AND** it computes absolute and relative error statistics for all seven supported operations
- **AND** it fails clearly if any expected output file or row count is missing

#### Scenario: Bit comparison against SFU_13 VHDL
- **WHEN** the Verilator comparison target runs on deterministic vectors
- **THEN** it runs the same input files through the SV and SFU_13 VHDL testbenches
- **AND** it reports input data, VHDL result, SV result, and pass/fail status
- **AND** every bit mismatch fails the target

### Requirement: Bit-Equivalent Voter Behavior
The SV voter implementation SHALL preserve the observable behavior of `Description/SFU_13/SFU/voter/voter.vhd`.

#### Scenario: Voter regression
- **WHEN** the Verilator test flow runs the voter-focused regression
- **THEN** representative `z1`, `z2`, and `z3` inputs produce the same `z` and `error` outputs as the SFU_13 VHDL voter behavior

### Requirement: SV Flow Documentation
The repository SHALL document how to run the SV RTL Verilator flow and interpret its generated files.

#### Scenario: Contributor reads SV simulation documentation
- **WHEN** a contributor opens `sv/sim/README.md`
- **THEN** the document lists the required Verilator environment, main Make targets, generated files, and relationship to `Description/SFU_13/ghdl`
