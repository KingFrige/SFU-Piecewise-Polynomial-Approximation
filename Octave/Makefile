OCTAVE ?= octave
OCTAVE_FLAGS ?= --no-gui --no-window-system --quiet
RUN_DIR := $(CURDIR)
MAKEFILE_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
LUT_DIR ?= $(RUN_DIR)/generated_luts
SELECTORS ?=

.PHONY: all test test-golden test-lut generate-luts clean

all: test

test: test-golden test-lut

test-golden:
	cd $(MAKEFILE_DIR) && $(OCTAVE) $(OCTAVE_FLAGS) test_sfu_golden_model.m

test-lut:
	cd $(MAKEFILE_DIR) && $(OCTAVE) $(OCTAVE_FLAGS) test_generate_lut_tables.m

generate-luts:
	cd $(MAKEFILE_DIR) && $(OCTAVE) $(OCTAVE_FLAGS) run_generate_lut_tables.m $(LUT_DIR) $(SELECTORS)

clean:
	rm -rf $(LUT_DIR)
