## 1. GHDL Flow Structure

- [x] 1.1 Create `Description/SFU_13/ghdl/` with generated-output ignores.
- [x] 1.2 Add a GNU Make entry point with default, data, analyze, elaborate, run, verify, and clean targets.

## 2. Linux Data and Result Scripts

- [x] 2.1 Add Linux-compatible input-data generation that writes the `SFU_Input_data/input_*.csv` files expected by `sfu_tb.vhd`.
- [x] 2.2 Add result analysis for the generated `output_*.csv` files and `SFU_Input_data/Error_analysis.txt`.
- [x] 2.3 Support an `N_DATA` override for quick smoke tests and full legacy-size runs.

## 3. GHDL Compilation and Simulation

- [x] 3.1 Encode the SFU_13 VHDL source list using Linux filename case and GHDL-compatible ordering.
- [x] 3.2 Use VHDL-93/Synopsys-compatible GHDL flags and run `sfu_tb` with a 15 ms stop time.
- [x] 3.3 Keep GHDL work files and simulation outputs inside the `ghdl/` workflow directory.

## 4. Documentation and Verification

- [x] 4.1 Document prerequisites, Make targets, generated files, and known GHDL differences in `Description/SFU_13/ghdl/README.md`.
- [x] 4.2 Run `make -C Description/SFU_13/ghdl N_DATA=1` as a smoke test.
- [x] 4.3 Run `openspec validate add-ghdl-sfu13-simulation --strict`.
