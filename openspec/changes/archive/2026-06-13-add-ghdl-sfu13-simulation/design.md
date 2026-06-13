## Context

`Description/SFU_13/Verification/testbench.tcl` drives the current flow by compiling VHDL-93 sources with ModelSim `vcom -93`, running `work.sfu_tb` for 15 ms, and leaving CSV outputs under `SFU_Input_data/`. The VHDL testbench opens input and output files through relative paths such as `./SFU_Input_data/input_sin.csv`, so the simulator working directory is part of the test contract.

The current Python input generator is not Linux portable because it shells out to Windows commands (`Xcopy`, `rmdir /s /q`, `mkdir`). The result analyzer is mostly reusable once the same `SFU_Input_data/` layout exists.

## Goals / Non-Goals

**Goals:**
- Add a GNU Make entry point under `Description/SFU_13/ghdl/`.
- Generate the same input CSV names and output directory layout expected by `sfu_tb.vhd`.
- Compile and elaborate SFU_13 RTL with GHDL using VHDL-93-compatible flags.
- Run `sfu_tb` for the ModelSim-equivalent 15 ms stop time.
- Run the existing style of Python result analysis and write `SFU_Input_data/Error_analysis.txt`.
- Keep generated simulator work files and CSV outputs out of version control.

**Non-Goals:**
- Do not modify SFU RTL behavior.
- Do not remove or replace the existing ModelSim/Questa verification directory.
- Do not make waveform viewing equivalent to `wave.do` in the first pass.
- Do not commit generated `SFU_Input_data` archives or GHDL work libraries.

## Decisions

1. Put the GHDL flow in `Description/SFU_13/ghdl/`.
   - Rationale: keeps the open-source flow separate from the legacy ModelSim scripts and satisfies the `sfu_tb.vhd` relative file paths by running simulation from the `ghdl/` directory.
   - Alternative considered: run from `Verification/` and replace `vsim` with `ghdl`; this would require changing or duplicating Windows-oriented scripts in place.

2. Add Linux-compatible helper scripts instead of directly invoking `Verification/2_Generate_input_data.py`.
   - Rationale: the existing generator uses Windows shell commands, but the data shape and numerical distributions can be preserved in a small Python 3 script.
   - Alternative considered: patch the existing generator to be cross-platform; that is broader and risks changing the ModelSim flow.

3. Use GHDL flags `--std=93c -fsynopsys`.
   - Rationale: the RTL is VHDL-93-oriented, and `sfu_tb.vhd` imports `ieee.std_logic_textio`, which GHDL accepts with Synopsys compatibility enabled.

4. Keep the VHDL source list explicit in the Makefile.
   - Rationale: the legacy Tcl already depends on a manual compile order; an explicit list makes differences visible and avoids accidental inclusion of backup files.
   - GHDL-specific ordering: compile `voter.vhd` before `Quadratic_Interpolator.vhd`, because GHDL requires directly instantiated entities to exist during analysis.

5. Preserve the 15 ms run limit.
   - Rationale: ModelSim uses `run 15 ms`; GHDL should use `--stop-time=15ms` to keep behavior aligned even though the testbench reports completion and then waits.

## Risks / Trade-offs

- Full `N_DATA=20000` simulation may be slow under GHDL mcode → keep the default equivalent to the legacy flow but allow `N_DATA` override through Make for quick smoke tests.
- Python result analysis currently imports Matplotlib even when not plotting → document the dependency or adjust the helper import pattern if needed.
- NumPy reference functions emit warnings for edge cases such as NaN, infinities, signed zero, and subnormals → treat this as inherited behavior from the existing verification flow.
- GHDL warnings about metavalue comparisons at time zero may appear → document them unless they correspond to failed simulation output or missing CSV files.
