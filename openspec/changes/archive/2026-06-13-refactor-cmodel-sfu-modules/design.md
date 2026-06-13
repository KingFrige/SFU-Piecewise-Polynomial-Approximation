## Context

`cmodel/src/sfu.c` currently combines public API functions, LUT file loading, metadata parsing, IEEE-754 helper logic, range reduction, interpolation, reconstruction, and exception handling. The hardware reference in `Description/SFU_13/SFU` separates these concerns into top-level `sfu.vhd`, `RRO`, `Quadratic_Interpolator`, `ROM`, `CLZ`/normalization, reconstruction logic, and `SFU_Exceptions`.

The refactor should preserve the existing public behavior and use the Octave golden model as the acceptance oracle. The goal is structural clarity, not a numerical algorithm change.

## Goals / Non-Goals

**Goals:**

- Keep `sfu.c/h` focused on top-level SFU evaluation and public API.
- Move LUT text-file parsing and path handling into a peripheral loader module.
- Add internal modules named after the hardware/paper structure: RRO, ROM, quadratic interpolator, reconstruction, and exceptions.
- Preserve current CLI behavior, Makefile targets, and test expectations.

**Non-Goals:**

- Do not redesign the LUT file format.
- Do not change coefficient generation in `lut/` or Octave LUT generation.
- Do not add TMR or fault-injection modeling to the C model.
- Do not change the public CLI output format.

## Decisions

### Keep ROM storage close to LUT loading

The hardware has a `ROM` block, while the C model also needs filesystem code to populate it. We will represent loaded coefficient tables as a ROM-like data structure used by the datapath, define the private table type in `sfu_internal.h`, and keep allocation, release, and generated text-file loading together in `sfu_lut_file.c`. This avoids separate files that only forward a few accessors.

Alternative considered: keep `sfu_load_luts()` in `sfu.c`. That preserves fewer files but continues mixing hardware datapath logic with file I/O.

### Keep top-level API stable

The existing API names `sfu_load_luts`, `sfu_free_luts`, and `sfu_eval_hex` will remain callable by the CLI and tests. Internally, `sfu_load_luts` may delegate to the LUT file loader so existing callers do not need a broad migration.

Alternative considered: replace the public API with `sfu_rom_load_from_dir`. That is cleaner architecturally but creates needless caller churn for this refactor.

### Use hardware-aligned module names

Source files will use names that make comparison to `Description/SFU_13/SFU` direct without creating one-file-per-helper overhead:

- `sfu_rro.*` for range reduction operations.
- `sfu_quadratic_interpolator.*` for X1/X2 selection, specialized squaring approximation, and fused accumulation behavior.
- `sfu_reconstruction.*` for result packing, exponent correction, and Octave-compatible IEEE-754 helper behavior.
- `sfu_exceptions.*` for final selector-specific exception resolution.
- `sfu_lut_file.c` for generated LUT directory parsing and loaded ROM table ownership.
- `sfu_internal.h` for private table types and small inline ROM table accessors.
- `sfu_support.c` for selector metadata, CLI hex parsing, and shared error formatting.

### Preserve Octave-compatible packing

`double_to_octave_hex32` behavior intentionally matches Golden-model `dec2hex754`, including nonstandard subnormal boundary behavior. It should move into reconstruction/IEEE helper code without changing semantics.

## Risks / Trade-offs

- API churn inside modules could hide a behavior change -> Mitigate by moving code mechanically first and running `make -C cmodel test` after each group.
- Table selector naming can become confusing because CLI selectors differ from hardware `selop_i` and ROM table ids -> Mitigate with explicit private types in `sfu_internal.h`.
- More files add Makefile maintenance -> Mitigate by listing object files explicitly and keeping module boundaries small.
