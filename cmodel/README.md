# SFU C Model

This directory contains a standalone C model for the SFU datapath. The hardware reference is `Description/SFU_13`; the acceptance oracle for the first implementation is `Octave/sfu_golden_model.m`.

The model consumes generated LUT directories with this layout:

```text
01_reci/LUTC0.txt
01_reci/LUTC1.txt
01_reci/LUTC2.txt
01_reci/metadata.txt
...
```

## Dependencies

The cmodel build and comparison flow requires GNU Make, a C11 compiler, Python 3, and GNU Octave. Octave is used through `../Octave` to generate LUTs and provide the golden comparison oracle.

Build and run deterministic tests:

```sh
make -C cmodel test
```

Run one evaluation:

```sh
make -C cmodel generate-luts
cmodel/build/sfu cmodel/build/octave_luts C23A36C1 10
```

Generate input txt files and compare against Octave:

```sh
make -C cmodel generate-data
make -C cmodel compare-octave
```

Input data is generated under `cmodel/build/input_data`; by default each function file contains 16 input hex values, one per line. Override the amount with `DATA_COUNT`, for example:

```sh
make -C cmodel compare-octave DATA_COUNT=1024
```

`compare-octave` depends on `generate-data`, so the input files are regenerated before each comparison. Comparison results are written under `cmodel/build/compare_octave` with this four-column txt format:

```text
input_hex golden_hex cmodel_hex error_status
```
