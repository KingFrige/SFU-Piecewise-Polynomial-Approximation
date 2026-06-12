## Context

The project has three relevant SFU references: `Golden-model/` and `Octave/` provide a bit-oriented software reference, `lut/` generates binary LUT files, and `Description/SFU_13/` is the hardware implementation to use as the datapath reference. `TODO.md` asks for a C model in `cmodel/src` with txt-based tests in `cmodel/tests`.

## Goals / Non-Goals

**Goals:**
- Implement a standalone C SFU model that evaluates IEEE-754 single-precision hex inputs for the supported SFU selectors.
- Load LUT files from the existing Octave/C LUT directory format.
- Match the Octave golden model on deterministic regression cases and bulk generated test data.
- Keep tests small enough for frequent regression while allowing larger generated datasets.

**Non-Goals:**
- Do not rewrite or regenerate VHDL.
- Do not replace the Octave golden model.
- Do not implement fault injection or voter behavior from redundant SFU variants.
- Do not claim full cycle-accurate simulation in the first version.

## Decisions

- Use `Description/SFU_13` as the hardware reference. This version includes the current SFU datapath and voter-era source tree, and it is the user-selected VHDL reference for this change.
- Use `Octave/sfu_golden_model.m` as the first acceptance oracle. It already mirrors the bit-level hardware flow and has known regression vectors.
- Load LUTs from files instead of embedding arrays. The model will consume directories like `Octave/generated_luts` or `lut/build/remez-compat`, keeping coefficient ownership in the existing LUT generators.
- Provide a small C API plus a CLI. The API supports tests and future integration; the CLI keeps txt data generation and one-off debugging simple.
- Use integer bit manipulation for LUT addressing, fixed-point interpolation, and IEEE-754 packing. Use host `float` only where the current Octave flow performs decimal range reduction for trig or hex/float conversion.

## Risks / Trade-offs

- [Risk] Octave and `Description/SFU_13` differ in some special-case details. → Mitigation: begin with Octave regression parity, document any VHDL divergence, then add focused tests as behavior is clarified.
- [Risk] Full bit-exact normalization is easy to get wrong. → Mitigation: implement incrementally with known vectors first, then broaden selectors and random data.
- [Risk] Large generated datasets can slow every edit/test cycle. → Mitigation: keep `make test` small and provide a separate generated-data comparison target.
- [Risk] Trig range reduction uses floating math in the current software reference. → Mitigation: match the Octave algorithm first, not libm ideal results.
