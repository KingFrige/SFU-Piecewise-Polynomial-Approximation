## 1. Octave Directory Migration

- [x] 1.1 Create `Octave/` and copy the golden-model helper `.m` files without changing `Golden-model/`.
- [x] 1.2 Preserve custom bit-string helper behavior required by the model.

## 2. Callable API and CLI

- [x] 2.1 Extract the script logic into `sfu_golden_model(input_hex, func)`.
- [x] 2.2 Add a command-line wrapper that evaluates one input/function pair and prints result hex plus decimal value.

## 3. Regression and Documentation

- [x] 3.1 Add a GNU Octave regression script with fixed expected hex outputs.
- [x] 3.2 Document the Octave execution and regression commands.
- [x] 3.3 Run the regression script successfully with GNU Octave.
