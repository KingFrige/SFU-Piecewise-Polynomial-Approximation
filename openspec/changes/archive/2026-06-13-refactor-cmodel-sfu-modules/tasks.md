## 1. Module Boundaries

- [x] 1.1 Move selector metadata and supported-selector helpers out of `sfu.c` into shared support code.
- [x] 1.2 Move coefficient table storage and ROM access helpers out of `sfu.c`.
- [x] 1.3 Move LUT directory/file parsing into `sfu_lut_file.c`.
- [x] 1.4 Keep public LUT load/free wrappers available for existing CLI and tests.

## 2. Hardware Datapath Modules

- [x] 2.1 Move range reduction logic into `sfu_rro.*`.
- [x] 2.2 Move quadratic interpolation logic into `sfu_quadratic_interpolator.*`.
- [x] 2.3 Move result reconstruction and Octave-compatible IEEE-754 packing into `sfu_reconstruction.*`.
- [x] 2.4 Move final exception handling into `sfu_exceptions.*`.
- [x] 2.5 Reduce `sfu.c/h` to top-level SFU evaluation orchestration and public API declarations.

## 3. Build, Tests, and Documentation

- [x] 3.1 Update `cmodel/Makefile` for the new source files.
- [x] 3.2 Update `cmodel/README.md` to describe the hardware-aligned source layout.
- [x] 3.3 Run deterministic cmodel tests and generated Octave comparison.
- [x] 3.4 Run `openspec validate refactor-cmodel-sfu-modules --strict`.
