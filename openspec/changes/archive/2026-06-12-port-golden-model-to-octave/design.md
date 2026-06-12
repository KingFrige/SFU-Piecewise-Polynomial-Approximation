## Context

`Golden-model/` contains an Octave-compatible hardware reference model for the SFU, but it is script-oriented and not presented as the Linux execution target. The model depends on custom bit-string helpers that intentionally shadow common names such as `bin2dec`, `dec2bin`, and `hex2bin`; these helpers are part of the hardware semantics and must be preserved.

## Goals / Non-Goals

**Goals:**

- Create an `Octave/` directory containing the migrated golden model and helpers.
- Add a stable Octave entry point for `input_hex` plus function selector.
- Keep outputs bit-oriented by returning the IEEE-754 result as an 8-character hex string.
- Add lightweight regression checks that can run on Linux with GNU Octave.
- Document how to run the migrated model.

**Non-Goals:**

- Do not change RTL files or ModelSim verification scripts.
- Do not rewrite the golden model in C/C++ in this change.
- Do not regenerate VHDL LUT files.
- Do not remove or mutate the existing `Golden-model/` reference.

## Decisions

1. Copy and adapt rather than move `Golden-model/`.
   - Rationale: keeping the original directory intact gives a direct comparison baseline and avoids breaking existing references.
   - Alternative considered: renaming `Golden-model/` to `Octave/`; rejected because it is disruptive and harder to review.

2. Add a function-style API around the script logic.
   - Rationale: a callable function enables tests and future batch integration while preserving the current single-vector algorithm.
   - Shape: `sfu_golden_model(input_hex, func)` returns `[result_hex, result_dec]`.

3. Keep helper names in the `Octave/` directory.
   - Rationale: the existing code relies on custom semantics for binary strings with fractional points. Renaming all helpers increases risk.
   - Mitigation: document that commands must run from `Octave/` or add it to the Octave path.

4. Use a small Octave regression script.
   - Rationale: the first migration should prove deterministic behavior without requiring ModelSim.
   - Fixture scope: known results from the current golden model for representative function/input pairs.

## Risks / Trade-offs

- Helper shadowing built-ins → run commands from `Octave/` and document the path requirement.
- Floating-point and rounding differences → compare hex strings, not only decimal values.
- Script-to-function extraction may accidentally change behavior → keep the original operation order and add regression cases.
- Existing model may not cover all IEEE exception paths → preserve behavior first; broaden cases in later changes.

## Migration Plan

1. Create `Octave/` and copy the golden-model helper files.
2. Extract `GoldenModel.m` logic into a callable Octave function.
3. Add a CLI wrapper for command-line use.
4. Add a regression script with fixed expected hex outputs.
5. Run regression with GNU Octave and document the commands.
