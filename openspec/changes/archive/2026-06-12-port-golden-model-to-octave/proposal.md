## Why

The current golden model is kept under `Golden-model/` as MATLAB/Octave-style scripts, but the repository does not provide a Linux-focused Octave entry point or migration boundary. Making Octave the explicit target gives contributors a reproducible open-source path for evaluating SFU reference behavior without MATLAB.

## What Changes

- Add an `Octave/` golden model directory that contains the Octave-targeted model and helper functions.
- Preserve the existing `Golden-model/` directory as the source reference during migration.
- Provide a documented Octave command-line entry point that accepts an IEEE-754 hex input and SFU function selector.
- Add a small regression fixture that checks migrated Octave outputs against known reference results.
- Document supported functions and local execution requirements.

## Capabilities

### New Capabilities

- `octave-golden-model`: Defines the Octave-targeted golden model interface, expected outputs, and regression checks for Linux execution.

### Modified Capabilities

None.

## Impact

- Affected code: new files under `Octave/`; optional documentation updates.
- Dependencies: GNU Octave on Linux.
- External behavior: no RTL behavior changes and no changes to existing `Description/SFU_##` verification flows.
