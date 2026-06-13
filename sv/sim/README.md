# SFU SystemVerilog Verilator Flow

This directory contains the Verilator simulation flow for the SystemVerilog SFU
RTL in `../SFU`. It mirrors the coverage shape of
`Description/SFU_13/ghdl`: per-function input CSVs, per-function output CSVs,
configurable data count, and NumPy error analysis. Each input contains eight
IEEE-754 special values plus exactly `N_DATA` generated values. Directed
sin/cos/exp2 boundaries consume part of `N_DATA`; random values fill the rest.

## Prerequisites

- GNU Make
- Python 3 with NumPy
- GNU Octave for LUT generation through `../../Octave`
- Verilator with SystemVerilog testbench support
- GHDL for direct bit comparison against `Description/SFU_13/SFU`

Load the expected Verilator environment when needed:

```sh
module load openEDA/verilator/v5.046
```

## Commands

Run the default verification:

```sh
make -C sv/sim
```

Run a quick one-generated-row smoke test:

```sh
make -C sv/sim smoke
```

Useful targets:

- `make data`: generate `build/SFU_Input_data/input_*.csv`
- `make luts`: generate Octave LUT text files under `build/octave_luts`
- `make build`: compile `tb_sfu.sv` and RTL with Verilator
- `make run`: run the SV testbench
- `make verify`: run simulation and write `Error_analysis.txt`
- `make compare`: compare SV results bit-for-bit against SFU_13 VHDL
- `make compare-cmodel`: optional comparison against the repository C model
- `make synth-check`: reject behavioral-only constructs and lint the RTL top
- `make voter`: run a focused voter regression
- `make clean`: remove generated build outputs

The testbench is SystemVerilog (`tb_sfu.sv`) and drives the DUT directly. The
flow does not use a project-maintained C++ stimulus harness.

`make compare` runs the same generated inputs through the SV and SFU_13 VHDL
testbenches. It writes four-column files under `build/compare_vhdl`:
`input_hex vhdl_hex sv_hex error_status`. Every mismatch fails the target.
