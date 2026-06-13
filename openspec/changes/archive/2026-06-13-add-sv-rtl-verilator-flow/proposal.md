## Why

The repository currently has a VHDL SFU_13 RTL implementation and a GHDL verification flow, but `TODO.md` calls for a Verilog/SystemVerilog RTL port that can be simulated with Verilator. Adding an SV implementation and a Verilator-based SV testbench makes the hardware model easier to use in modern open RTL flows while preserving the SFU_13 behavior as the reference.

## What Changes

- Add `sv/SFU/` with a SystemVerilog/Verilog port of the `Description/SFU_13/SFU` hardware structure.
- Keep `sv/SFU/` synthesizable: fixed-width logic only, with no `real`-valued reconstruction or simulation-only math.
- Add `sv/sim/` with a Verilator flow driven by an SV testbench, not a C++ harness.
- Port the SFU_13 verification style from `Description/SFU_13/ghdl/`: generated per-function input data, per-function output files, configurable data count, result analysis, and cleanup.
- Preserve the existing SFU_13 selector and datapath behavior, including bit-equivalent voter behavior.
- Document the SV RTL structure, Verilator dependency, Make targets, generated files, and expected validation workflow.

## Capabilities

### New Capabilities

- `sv-rtl-verilator-flow`: Defines the SystemVerilog RTL port and Verilator simulation flow for SFU_13.

### Modified Capabilities

## Impact

- Affected code: new `sv/SFU/` RTL files, new `sv/sim/` simulation files, and repository documentation.
- Dependencies: Verilator with SystemVerilog testbench support, GHDL for direct VHDL comparison, GNU Make, Python 3, and Octave for LUT generation.
- Existing VHDL, GHDL, Octave, LUT, and C model flows remain unchanged.
