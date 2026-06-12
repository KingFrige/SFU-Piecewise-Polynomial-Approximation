CC ?= gcc
CFLAGS ?= -std=c11 -O2 -Wall -Wextra -Wpedantic
LDFLAGS ?=
LDLIBS ?= -lm
MPFR_CFLAGS ?= $(shell pkg-config --cflags mpfr gmp 2>/dev/null)
MPFR_LDLIBS ?= $(shell pkg-config --libs mpfr gmp 2>/dev/null)
HAVE_MPFR := $(shell pkg-config --exists mpfr gmp && echo yes || echo no)

BUILD_DIR := build
C_LUT_DIR ?= $(BUILD_DIR)/remez-compat
OCTAVE_LUT_DIR ?= $(BUILD_DIR)/octave_generated_luts
REMEZ_COMPAT_LUT_DIR ?= $(BUILD_DIR)/remez-compat
REMEZ_LUT_DIR ?= $(BUILD_DIR)/remez
PDF_QUANT_LUT_DIR ?= $(BUILD_DIR)/pdf-quant
MPFR_REMEZ_LUT_DIR ?= $(BUILD_DIR)/mpfr-remez

.PHONY: all clean test smoke-files generate-octave compare-octave mpfr generate-variants compare-generated-variants compare-variants compare

all: $(BUILD_DIR)/coeffgen $(BUILD_DIR)/test_coeffgen $(BUILD_DIR)/compare_lut_dirs $(BUILD_DIR)/compare_variants

$(BUILD_DIR):
	mkdir -p $@

$(BUILD_DIR)/coeffgen.o: coeffgen.c coeffgen.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/coeffgen_cli.o: coeffgen_cli.c coeffgen.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/test_coeffgen.o: test_coeffgen.c coeffgen.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/compare_lut_dirs.o: compare_lut_dirs.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/compare_variants.o: compare_variants.c coeffgen.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/coeffgen.mpfr.o: coeffgen.c coeffgen.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(MPFR_CFLAGS) -DCOEFFGEN_HAVE_MPFR -c $< -o $@

$(BUILD_DIR)/coeffgen_mpfr.o: coeffgen_mpfr.c coeffgen.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(MPFR_CFLAGS) -DCOEFFGEN_HAVE_MPFR -c $< -o $@

$(BUILD_DIR)/coeffgen_cli.mpfr.o: coeffgen_cli.c coeffgen.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(MPFR_CFLAGS) -c $< -o $@

$(BUILD_DIR)/compare_variants.mpfr.o: compare_variants.c coeffgen.h | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(MPFR_CFLAGS) -DCOEFFGEN_HAVE_MPFR -c $< -o $@

$(BUILD_DIR)/coeffgen: $(BUILD_DIR)/coeffgen.o $(BUILD_DIR)/coeffgen_cli.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD_DIR)/coeffgen_mpfr: $(BUILD_DIR)/coeffgen.mpfr.o $(BUILD_DIR)/coeffgen_mpfr.o $(BUILD_DIR)/coeffgen_cli.mpfr.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) $(MPFR_LDLIBS) -o $@

$(BUILD_DIR)/test_coeffgen: $(BUILD_DIR)/coeffgen.o $(BUILD_DIR)/test_coeffgen.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD_DIR)/compare_lut_dirs: $(BUILD_DIR)/compare_lut_dirs.o
	$(CC) $(LDFLAGS) $^ -o $@

$(BUILD_DIR)/compare_variants: $(BUILD_DIR)/coeffgen.o $(BUILD_DIR)/compare_variants.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@

$(BUILD_DIR)/compare_variants_mpfr: $(BUILD_DIR)/coeffgen.mpfr.o $(BUILD_DIR)/coeffgen_mpfr.o $(BUILD_DIR)/compare_variants.mpfr.o
	$(CC) $(LDFLAGS) $^ $(LDLIBS) $(MPFR_LDLIBS) -o $@

test: all smoke-files
	$(BUILD_DIR)/test_coeffgen

smoke-files: all
	rm -rf $(C_LUT_DIR)
	$(BUILD_DIR)/coeffgen $(C_LUT_DIR)
	test -f $(C_LUT_DIR)/06_exp/LUTC0.txt
	test -f $(C_LUT_DIR)/06_exp/LUTC1.txt
	test -f $(C_LUT_DIR)/06_exp/LUTC2.txt
	test -f $(C_LUT_DIR)/06_exp/metadata.txt
	test -f $(C_LUT_DIR)/09_sin/LUTC0.txt
	test -f $(C_LUT_DIR)/10_cos/LUTC2.txt
	test "$$(find $(C_LUT_DIR) -mindepth 1 -maxdepth 1 -type d | wc -l)" -eq 10
	test "$$(find $(C_LUT_DIR) -name '*.txt' | wc -l)" -eq 40
	awk 'length != 29 || /[^01]/ { exit 1 }' $(C_LUT_DIR)/*/LUTC0.txt
	awk 'length != 20 || /[^01]/ { exit 1 }' $(C_LUT_DIR)/*/LUTC1.txt
	awk 'length != 14 || /[^01]/ { exit 1 }' $(C_LUT_DIR)/*/LUTC2.txt

generate-octave: all
	rm -rf $(OCTAVE_LUT_DIR)
	$(MAKE) -f ../Octave/Makefile generate-luts LUT_DIR=$(abspath $(OCTAVE_LUT_DIR))

compare-octave: all smoke-files generate-octave
	$(BUILD_DIR)/compare_lut_dirs $(OCTAVE_LUT_DIR) $(C_LUT_DIR)

ifeq ($(HAVE_MPFR),yes)
mpfr: $(BUILD_DIR)/coeffgen_mpfr $(BUILD_DIR)/compare_variants_mpfr
else
mpfr:
	@echo "MPFR/GMP development libraries are unavailable; build skipped."
endif

generate-variants: all
	rm -rf $(REMEZ_COMPAT_LUT_DIR) $(REMEZ_LUT_DIR) $(PDF_QUANT_LUT_DIR) $(MPFR_REMEZ_LUT_DIR)
	$(BUILD_DIR)/coeffgen --variant=remez-compat $(REMEZ_COMPAT_LUT_DIR)
	$(BUILD_DIR)/coeffgen --variant=remez $(REMEZ_LUT_DIR)
	$(BUILD_DIR)/coeffgen --variant=pdf-quant $(PDF_QUANT_LUT_DIR)
ifeq ($(HAVE_MPFR),yes)
	$(MAKE) mpfr
	$(BUILD_DIR)/coeffgen_mpfr --variant=mpfr-remez $(MPFR_REMEZ_LUT_DIR)
else
	@echo "Skipping mpfr-remez variant; MPFR/GMP development libraries are unavailable."
endif

compare-generated-variants: all
ifeq ($(HAVE_MPFR),yes)
	$(BUILD_DIR)/compare_variants_mpfr $(REMEZ_COMPAT_LUT_DIR) \
		remez-compat $(REMEZ_COMPAT_LUT_DIR) \
		remez $(REMEZ_LUT_DIR) \
		pdf-quant $(PDF_QUANT_LUT_DIR) \
		mpfr-remez $(MPFR_REMEZ_LUT_DIR)
else
	$(BUILD_DIR)/compare_variants $(REMEZ_COMPAT_LUT_DIR) \
		remez-compat $(REMEZ_COMPAT_LUT_DIR) \
		remez $(REMEZ_LUT_DIR) \
		pdf-quant $(PDF_QUANT_LUT_DIR)
endif

compare-variants: generate-variants
	$(MAKE) compare-generated-variants

compare: generate-octave generate-variants
	$(BUILD_DIR)/compare_lut_dirs $(OCTAVE_LUT_DIR) $(REMEZ_COMPAT_LUT_DIR)
	$(MAKE) compare-generated-variants

clean:
	rm -rf $(BUILD_DIR)
