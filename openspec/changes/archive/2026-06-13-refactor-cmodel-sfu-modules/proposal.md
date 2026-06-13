## Why

The cmodel currently concentrates LUT file loading, selector helpers, IEEE-754 utilities, and SFU datapath logic in `cmodel/src/sfu.c`, making it hard to compare the implementation against the paper and `Description/SFU_13/SFU` hardware blocks. Refactoring the source layout will make the C model easier to audit and extend while preserving bit-exact behavior against the Octave golden model.

## What Changes

- Split non-hardware helper code out of `sfu.c/h`, including LUT file parsing, path handling, text parsing, and selector utility code.
- Organize the C datapath modules around the hardware and paper terminology: RRO, quadratic interpolator, ROM, reconstruction/normalization, and SFU exceptions.
- Keep `sfu.c/h` focused on the top-level SFU algorithm and public evaluation API.
- Update build rules, tests, and documentation for the new source layout.
- Preserve existing CLI behavior, generated-data comparison behavior, and Octave golden equivalence.

## Capabilities

### New Capabilities

### Modified Capabilities

- `sfu-cmodel`: Adds structural requirements for the C model source layout without changing its external evaluation behavior.

## Impact

- Affected code: `cmodel/src`, `cmodel/Makefile`, `cmodel/tests`, and `cmodel/README.md`.
- Public CLI and output format remain unchanged.
- No new runtime dependencies are introduced.
