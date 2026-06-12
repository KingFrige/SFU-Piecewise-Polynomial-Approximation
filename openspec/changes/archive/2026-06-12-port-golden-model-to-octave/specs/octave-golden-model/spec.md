## ADDED Requirements

### Requirement: Octave golden model directory
The repository SHALL provide an `Octave/` directory containing the GNU Octave-targeted SFU golden model and all helper functions required to run it on Linux.

#### Scenario: Directory contains runnable model
- **WHEN** a contributor inspects `Octave/`
- **THEN** the directory contains the migrated model entry point and the helper `.m` files needed by that entry point

### Requirement: Callable golden model API
The Octave golden model SHALL expose a callable API that accepts an IEEE-754 single-precision input encoded as hexadecimal text and an SFU function selector, then returns the result as hexadecimal text.

#### Scenario: Default migrated example
- **WHEN** the model is called with input `C23A36C1` and function selector `10`
- **THEN** it returns result hex `BF5777FD`

### Requirement: Command-line execution
The Octave golden model SHALL provide a documented command-line way to evaluate one input/function pair without editing source files.

#### Scenario: CLI invocation
- **WHEN** a contributor runs the documented Octave command with input hex and function selector arguments
- **THEN** the command prints the result hex and decimal interpretation

### Requirement: Regression verification
The Octave migration SHALL include a regression script that validates fixed input/function cases against expected result hex strings.

#### Scenario: Passing regression
- **WHEN** the regression script is run with GNU Octave
- **THEN** it exits successfully after checking all expected result hex strings
