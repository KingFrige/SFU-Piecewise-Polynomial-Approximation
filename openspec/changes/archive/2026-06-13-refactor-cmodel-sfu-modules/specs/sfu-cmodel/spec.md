## ADDED Requirements

### Requirement: Hardware-Aligned C Source Organization
The C SFU model SHALL keep hardware datapath logic separated from peripheral file and parsing helpers.

#### Scenario: SFU top-level source stays hardware-focused
- **WHEN** a contributor inspects `cmodel/src/sfu.c` and `cmodel/src/sfu.h`
- **THEN** those files expose the top-level SFU evaluation API and hardware datapath orchestration
- **AND** they do not contain LUT text-file parsing or filesystem path construction logic

#### Scenario: Peripheral LUT loading is isolated
- **WHEN** the cmodel loads generated LUT text files from disk
- **THEN** file layout discovery, metadata parsing, binary row parsing, and memory ownership for loaded tables are implemented outside `sfu.c`

#### Scenario: Module names align with SFU hardware structure
- **WHEN** a contributor inspects `cmodel/src`
- **THEN** source modules use names that correspond to the paper and `Description/SFU_13/SFU` structure, including RRO, quadratic interpolation, ROM, reconstruction or normalization, and SFU exceptions

#### Scenario: Refactor preserves golden behavior
- **WHEN** the user runs deterministic tests and Octave generated-data comparison
- **THEN** the C model results remain bit-equivalent to the Octave golden model for the tested vectors
