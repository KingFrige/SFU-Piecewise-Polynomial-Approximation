# Repository Guidelines

## Project Structure & Module Organization

This repository contains an IEEE-754 Special Function Unit implemented in VHDL, with reference and verification utilities.

- `Description/SFU_00`, `Description/SFU_11`, `Description/SFU_12`, and `Description/SFU_13` are versioned SFU implementations. Prefer the newest version, currently `SFU_13`, unless comparing revisions.
- `Description/SFU_##/SFU/` contains RTL: top-level `sfu.vhd`, testbenches `sfu_tb*.vhd`, shared `Components/`, range reduction in `RRO/`, interpolation logic in `Quadratic_Interpolator/`, LUTs in `Single_LUTS/`, and voter/fused accumulation blocks.
- `Description/SFU_##/Verification/` contains ModelSim/Questa scripts, waveform setup, Python data generation, and result analysis.
- `Golden-model/` contains MATLAB/Octave scripts used as reference models and LUT utilities.

## Build, Test, and Development Commands

There is no top-level build system. Run commands from the selected version's `Verification` directory:

```sh
cd Description/SFU_13/Verification
python 2_Generate_input_data.py
vsim -c -do testbench.tcl
python 3_Verification_results.py
```

`2_Generate_input_data.py` creates input CSVs under `SFU_Input_data/`. `testbench.tcl` compiles VHDL-93 sources with `vcom -93`, runs `work.sfu_tb` for 15 ms, and writes output CSVs. `3_Verification_results.py` compares simulator output against NumPy references and writes `SFU_Input_data/Error_analysis.txt`.

On Windows with ModelSim-Altera in the expected path, `python 1_launch_modelsim_simulation.py` runs the full flow. For the reference model, run `octave GoldenModel.m` from `Golden-model/`.

## Coding Style & Naming Conventions

Keep VHDL compatible with VHDL-93 and follow the existing aligned, tab-indented style. Preserve established naming patterns: input ports end in `_i`, output ports in `_o`, internal signals often use `s_`, and constants use `C_`. Name testbenches with `_tb.vhd`; name LUT files as `LUT_C<order>_<function>.vhd`.

Python verification scripts are simple procedural utilities. Keep generated file paths relative to the active `Verification` directory and avoid introducing hidden global dependencies.

## Testing Guidelines

Use ModelSim-Altera or QuestaSim for RTL verification. Test changes against the relevant `sfu_tb.vhd` through `testbench.tcl`, then review `Error_analysis.txt` for all operations: `sin`, `cos`, `rsqrt`, `log2`, `ex2`, `rcp`, and `sqrt`. Include edge cases such as NaN, infinities, signed zero, and subnormals when modifying exception handling.

## Commit & Pull Request Guidelines

The history uses short descriptive messages such as `Update README.md` and targeted fixes. Prefer concise imperative commits with a scope, for example `Fix SFU_13 exp2 negative overflow`. Pull requests should state the affected SFU version, summarize RTL or model changes, list simulator commands run, and include key error metrics when behavior changes.

## Generated Files & Configuration

Do not commit simulator work libraries, waveform dumps, or regenerated `SFU_Input_data` archives unless they are intentional verification baselines. The launch script contains a hard-coded Windows ModelSim path; document local path changes instead of committing machine-specific edits.
