# SFU_13 GHDL Verification

This directory provides a GNU Make and GHDL flow for running the existing
`Description/SFU_13/SFU/sfu_tb.vhd` testbench without ModelSim or QuestaSim.

## Prerequisites

- GNU Make
- GHDL with VHDL-93 and Synopsys compatibility support
- Python 3
- NumPy

On Debian or Ubuntu:

```sh
sudo apt-get install make ghdl python3 python3-numpy
```

## Commands

Run the full flow:

```sh
make
```

The default run uses `N_DATA=20000`, matching the legacy ModelSim input
generator. Run a quick smoke test with one random value per operation:

```sh
make N_DATA=1
```

Useful targets:

- `make data`: generate `SFU_Input_data/input_*.csv`
- `make analyze`: analyze the SFU_13 VHDL sources with GHDL
- `make elaborate`: elaborate `sfu_tb`
- `make run`: generate inputs and run the testbench
- `make verify`: run simulation and write error metrics
- `make clean`: remove GHDL work files and generated CSV outputs

`make verify` checks that each simulator output file contains the expected
`N_DATA` result rows after the inherited special IEEE-754 cases. If GHDL stops
before `sfu_tb` finishes, result analysis exits with an error instead of writing
misleading partial metrics.

## Generated Files

The flow writes generated files under this directory:

- `work/`: GHDL work library files
- `SFU_Input_data/input_*.csv`: generated input vectors
- `SFU_Input_data/output_*.csv`: testbench output vectors
- `SFU_Input_data/Error_analysis.txt`: NumPy reference error metrics

These generated files are ignored by Git.

## Notes

The testbench opens files through relative paths such as
`./SFU_Input_data/input_sin.csv`, so run Make from this directory or use
`make -C Description/SFU_13/ghdl`.

The GHDL source list follows the ModelSim Tcl flow, with two Linux/GHDL
adjustments:

- `../SFU/RRO/rro.vhd` uses the actual lowercase filename.
- `../SFU/voter/voter.vhd` is analyzed before
  `Quadratic_Interpolator.vhd` because GHDL requires directly instantiated
  entities to be present during analysis.

GHDL may print numeric_std metavalue warnings at time zero. Those warnings are
expected for the current testbench initialization pattern; failed file opens,
missing outputs, or a missing `Error_analysis.txt` indicate a real flow failure.
