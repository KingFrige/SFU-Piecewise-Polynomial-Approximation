CC ?= gcc
PYTHON ?= python3
OCTAVE ?= octave
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wpedantic
LDFLAGS ?=
LDLIBS ?= -lm

BUILD_DIR := build
LUT_DIR ?= $(BUILD_DIR)/octave_luts
DATA_DIR ?= $(BUILD_DIR)/input_data
RESULT_DIR ?= $(BUILD_DIR)/compare_octave
TEST_TMP_DIR ?= $(BUILD_DIR)/test_tmp
DATA_COUNT ?= 16

SFU_SRCS := \
	src/sfu.c \
	src/sfu_exceptions.c \
	src/sfu_lut_file.c \
	src/sfu_quadratic_interpolator.c \
	src/sfu_reconstruction.c \
	src/sfu_rro.c \
	src/sfu_support.c
SFU_OBJS := $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SFU_SRCS))

.PHONY: all clean generate-luts test generate-data compare-octave

all: $(BUILD_DIR)/sfu $(BUILD_DIR)/test_sfu

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/%.o: src/%.c src/*.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/sfu_cli.o: src/sfu_cli.c src/sfu.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/test_sfu.o: tests/test_sfu.c src/sfu.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -Isrc -c $< -o $@

$(BUILD_DIR)/sfu: $(SFU_OBJS) $(BUILD_DIR)/sfu_cli.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD_DIR)/test_sfu: $(SFU_OBJS) $(BUILD_DIR)/test_sfu.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

generate-luts:
	$(MAKE) -C ../Octave generate-luts LUT_DIR=$(abspath $(LUT_DIR))

test: all generate-luts
	$(BUILD_DIR)/test_sfu $(LUT_DIR) $(TEST_TMP_DIR)

generate-data: all generate-luts
	$(PYTHON) tests/compare_octave.py --generate-only --sfu $(abspath $(BUILD_DIR)/sfu) --lut-dir $(abspath $(LUT_DIR)) --data-dir $(DATA_DIR) --result-dir $(abspath $(RESULT_DIR)) --count $(DATA_COUNT) --octave $(OCTAVE)

compare-octave: generate-data
	$(PYTHON) tests/compare_octave.py --sfu $(abspath $(BUILD_DIR)/sfu) --lut-dir $(abspath $(LUT_DIR)) --data-dir $(DATA_DIR) --result-dir $(abspath $(RESULT_DIR)) --count $(DATA_COUNT) --octave $(OCTAVE)

clean:
	rm -rf $(BUILD_DIR)
