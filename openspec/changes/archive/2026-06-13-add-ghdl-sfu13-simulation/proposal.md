## Why

The SFU_13 RTL verification flow currently depends on ModelSim/Questa scripts and Windows-oriented helper commands, which blocks Linux-only verification when ModelSim is unavailable. A GHDL-based flow gives contributors a local open-source path to generate inputs, run the existing `sfu_tb`, and review the same error metrics.

## What Changes

- Add a `Description/SFU_13/ghdl/` verification entry point for GNU Make and GHDL.
- Reuse the existing SFU_13 VHDL testbench and Python result analysis semantics.
- Provide Linux-compatible input-data generation that writes the `SFU_Input_data/` layout expected by `sfu_tb.vhd`.
- Compile the SFU_13 VHDL sources with GHDL-compatible flags and ordering.
- Run `work.sfu_tb` with an equivalent simulation stop time and produce output CSV files.
- Document prerequisites, commands, generated outputs, and known simulator differences.

## Capabilities

### New Capabilities
- `ghdl-sfu13-simulation`: GHDL-based build, run, and result-analysis flow for `Description/SFU_13`.

### Modified Capabilities
- None.

## Impact

- Adds files under `Description/SFU_13/ghdl/`.
- Uses existing `Description/SFU_13/SFU/` RTL and `sfu_tb.vhd` without changing RTL behavior.
- Reuses or mirrors `Description/SFU_13/Verification/` data generation and result analysis behavior.
- Requires GHDL, GNU Make, Python 3, NumPy, and Matplotlib-compatible Python imports for the verification scripts.
