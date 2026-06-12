# Octave Golden Model

This directory contains the GNU Octave-targeted wrappers, tests, and LUT generation commands. Shared golden-model helpers are loaded from `../Golden-model/` at runtime, so files such as `coeff.m`, `loadLUTs.m`, `bin2dec.m`, `dec2bin.m`, and `hex2bin.m` are maintained only once.

## Function Selectors

The callable model uses the original golden-model function selectors:

| Selector | Operation |
| --- | --- |
| `1` | reciprocal, `1/x` |
| `2` | square root, `sqrt(x)` |
| `4` | reciprocal square root, `1/sqrt(x)` |
| `6` | power of two, `2^x` |
| `7` | binary logarithm, `log2(x)` |
| `9` | sine, `sin(x)` |
| `10` | cosine, `cos(x)` |

Selectors `3`, `5`, and `8` are internal LUT variants selected by the model.

## Single Evaluation

```sh
cd Octave
octave --no-gui --no-window-system --quiet run_sfu_golden_model.m C23A36C1 10
```

Expected output:

```text
result_hex=BF5777FD
result_dec=-0.841674625873566
```

The callable API is:

```octave
[result_hex, result_dec] = sfu_golden_model("C23A36C1", 10);
```

## Regression

```sh
cd Octave
make test
```

Run only the golden-model regression:

```sh
make test-golden
```

## LUT Table Generation

Generate complete `LUTC0.txt`, `LUTC1.txt`, and `LUTC2.txt` bit tables for every LUT selector:

```sh
cd Octave
make generate-luts
```

By default this writes `generated_luts/` under the directory where `make` is executed. Override the destination with `LUT_DIR=/path/to/output`.

Generate only selected LUT tables by passing selector numbers after the output directory:

```sh
make generate-luts SELECTORS="1 2 9 10"
```

The output directory contains one subdirectory per selector, for example `09_sin/`, with:

- `LUTC0.txt`: complete 29-bit C0 bus rows
- `LUTC1.txt`: complete 20-bit C1 bus rows
- `LUTC2.txt`: complete 14-bit C2 bus rows
- `metadata.txt`: selector, function name, row count, widths, and `m`

Run the LUT generation test:

```sh
make test-lut
```
