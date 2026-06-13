## 1. RTL Structure

- [x] 1.1 Create `sv/SFU` directory structure and source list for the SV RTL.
- [x] 1.2 Port the SFU top-level, RRO, CLZ, reconstruction, and exception datapath modules from SFU_13 behavior.
- [x] 1.3 Port the quadratic interpolator, squaring, fused accumulation, Booth/CSA helpers, and bit-equivalent voter behavior.
- [x] 1.4 Implement ROM/LUT access using generated binary text LUT files instead of hand-porting every VHDL LUT entity.

## 2. Verilator SV Simulation Flow

- [x] 2.1 Create `sv/sim` with a Verilator Makefile that builds and runs an SV testbench without a project C++ stimulus harness.
- [x] 2.2 Port GHDL-style input data generation for all seven supported operations with configurable `N_DATA`.
- [x] 2.3 Implement an SV testbench that reads per-function input files, drives RRO plus SFU, and writes per-function output files.
- [x] 2.4 Port result analysis for absolute/relative error statistics and missing-row detection.

## 3. Reference Comparison and Focused Regressions

- [x] 3.1 Add a deterministic bit-comparison task against the SFU_13 VHDL reference.
- [x] 3.2 Add a focused voter regression that checks SV outputs against SFU_13 voter behavior.
- [x] 3.3 Add smoke targets for fast low-count simulation and default verification targets for broader coverage.

## 4. Documentation and Validation

- [x] 4.1 Document the SV RTL layout, Verilator environment, Make targets, generated files, and relation to the GHDL flow.
- [x] 4.2 Run RTL formatting-neutral source checks where available.
- [x] 4.3 Run Verilator build/smoke verification when Verilator is available.
- [x] 4.4 Run `openspec validate add-sv-rtl-verilator-flow --strict`.

## 5. Review Corrections

- [x] 5.1 Remove all non-synthesizable `real`-valued computation from `sv/SFU`.
- [x] 5.2 Match SFU_13 exp2 RRO special encodings and sin/cos boundary exceptions bit-for-bit.
- [x] 5.3 Ensure generated Verilator outputs are ignored and absent from the source patch.
- [x] 5.4 Run a synthesis-oriented lint scan that rejects behavioral-only constructs in `sv/SFU`.
- [x] 5.5 Keep generated inputs at exactly eight special rows plus `N_DATA` test rows so legacy-size VHDL comparison completes.
