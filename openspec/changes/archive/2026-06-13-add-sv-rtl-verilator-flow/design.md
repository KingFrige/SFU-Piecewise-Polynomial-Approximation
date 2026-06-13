## Context

`TODO.md` requests a Verilog/SystemVerilog version of the SFU RTL under `sv/SFU` and a Verilator simulation environment under `sv/sim`. The closest hardware reference is `Description/SFU_13/SFU`, with a GHDL flow in `Description/SFU_13/ghdl` that already provides Linux-friendly input generation, source ordering, simulation, output CSVs, and error analysis.

The implementation must preserve SFU_13 behavior first. The SV testbench should be a SystemVerilog testbench accepted by Verilator; the flow should not depend on a hand-written C++ harness. The existing C model and Octave golden model remain useful reference oracles for bit-level comparison and deterministic vectors.

## Goals / Non-Goals

**Goals:**

- Add an `sv/SFU` RTL tree that mirrors the SFU_13 hardware structure: RRO, top-level SFU, quadratic interpolator, ROM/LUTs, CLZ, reconstruction, exceptions, squaring, fused accumulation, and voter.
- Keep every module under `sv/SFU` synthesizable and implement reconstruction through the SFU_13 fixed-width CLZ, rounding, and exponent-correction datapath.
- Add an `sv/sim` Verilator flow using an SV testbench as the driver.
- Port the GHDL verification shape: generated `input_*.csv`, produced `output_*.csv`, configurable `N_DATA`, result analysis, and `clean`.
- Keep the VHDL voter behavior bit-equivalent, even if the logic is not a conventional per-bit majority voter.
- Use small-step validation so compilation, smoke vectors, generated-data simulation, and analysis can be run independently.

**Non-Goals:**

- Do not replace or remove the VHDL/GHDL flow.
- Do not introduce a C++ Verilator harness for stimulus driving.
- Do not redesign the SFU algorithm, LUT format, or coefficient generation.
- Do not fix VHDL behavioral quirks unless a later change explicitly asks for a functional change.

## Decisions

### Use a pure SV testbench with Verilator timing

`sv/sim/tb_sfu.sv` will drive inputs, instantiate the SV RRO and SFU modules, wait the same single-cycle combinational cadence used by the VHDL testbench, and write CSV outputs. Verilator will be invoked with timing support for `#` delays/event waits as needed.

Alternative considered: C++ harness driving `eval()`. That is common for Verilator, but the requested flow explicitly requires an SV testbench.

### Preserve the SFU_13 external partition

The VHDL `sfu_tb.vhd` instantiates `rro` outside `sfu` and sends RRO output into `sfu` only for sin, cos, and exp2. The SV flow will keep that partition so behavior and waveform-level debugging stay comparable.

Alternative considered: fold RRO into `sfu.sv`. That would create a cleaner standalone top-level, but it would change the reference boundary and make direct SFU_13 comparison harder.

### Use generated text LUTs instead of 30 handwritten LUT modules

The SV ROM should read the existing generated binary text LUTs (`LUTC0.txt`, `LUTC1.txt`, `LUTC2.txt`) with `$readmemb`. This keeps the SV RTL aligned with `Octave/generated_luts` and `lut/` output and avoids mechanically porting many near-identical VHDL LUT entities.

The LUT root is a compile-time module parameter. Runtime plusargs remain confined to the SV testbench.

Alternative considered: port every `Single_LUTS/LUT_C*.vhd` file to SV. That is closer to the VHDL file layout but increases maintenance cost and duplicates data already generated elsewhere.

### Preserve fixed-width RTL behavior

The SV port uses fixed-width integer arithmetic throughout. The top-level reconstruction follows `sfu.vhd` signal widths and ordering for log2 adjustment, normalization, rounding, exponent correction, and exception handling. RRO floating-point helpers are synthesizable ports of the VHDL multiplier and adder/subtractor behavior.

Alternative considered: use `real` arithmetic and repack the result. That is useful as a behavioral model but is not synthesizable and is not bit-equivalent on all rounding and exceptional cases.

## Risks / Trade-offs

- SV `$readmemb` path handling differs across run directories -> Set the ROM root as a compile-time parameter and run from `sv/sim`.
- Verilator may reject some testbench constructs supported by simulators -> Keep `tb_sfu.sv` simple: file I/O, plusargs, loops, delays, and `$finish`.
- Structural arithmetic shortcuts may diverge from VHDL on corner cases -> Run the same vectors through GHDL and Verilator and fail on every bit mismatch.
- RRO trig depends on VHDL floating-point helper behavior -> Implement and validate RRO early with targeted sin/cos/exp2 vectors before broad generated-data runs.
- Existing voter behavior may be nonstandard -> Preserve bit-equivalent logic in the SV port and add a focused voter test.
