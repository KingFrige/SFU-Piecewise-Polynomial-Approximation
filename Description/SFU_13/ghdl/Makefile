GHDL ?= ghdl
PYTHON ?= python3
N_DATA ?= 20000
STOP_TIME ?= 15ms

GHDLFLAGS ?= --std=93c -fsynopsys
WORKDIR := work
DATA_DIR := SFU_Input_data
SFU_DIR := ../SFU

VHDL_SOURCES := \
	$(SFU_DIR)/RRO/Components/fp_leading_zeros_and_shift.vhd \
	$(SFU_DIR)/RRO/Components/right_shifter.vhd \
	$(SFU_DIR)/RRO/Components/add_sub.vhd \
	$(SFU_DIR)/RRO/Components/multFP.vhd \
	$(SFU_DIR)/RRO/RRO_trig.vhd \
	$(SFU_DIR)/RRO/rro.vhd \
	$(SFU_DIR)/Components/CLZ.vhd \
	$(SFU_DIR)/Components/SFU_Exceptions.vhd \
	$(SFU_DIR)/fused_accm_tree/Booth_PP.vhd \
	$(SFU_DIR)/fused_accm_tree/CSA_4_2.vhd \
	$(SFU_DIR)/fused_accm_tree/fused_accum_tree.vhd \
	$(SFU_DIR)/voter/voter.vhd \
	$(SFU_DIR)/Quadratic_Interpolator/Single_LUTS/LUT_C0_cos.vhd \
	$(SFU_DIR)/Quadratic_Interpolator/Single_LUTS/LUT_C0_exp.vhd \
	$(SFU_DIR)/Quadratic_Interpolator/Single_LUTS/LUT_C0_ln2.vhd \
	$(SFU_DIR)/Quadratic_Interpolator/Single_LUTS/LUT_C0_ln2e0.vhd \
	$(SFU_DIR)/Quadratic_Interpolator/Single_LUTS/LUT_C0_reci.vhd \
	$(SFU_DIR)/Quadratic_Interpolator/Single_LUTS/LUT_C0_reci_sqrt_1_2.vhd \
	$(SFU_DIR)/Quadratic_Interpolator/Single_LUTS/LUT_C0_reci_sqrt_2_4.vhd \
	$(SFU_DIR)/Quadratic_Interpolator/Single_LUTS/LUT_C0_sin.vhd \
	$(SFU_DIR)/Quadratic_Interpolator/Single_LUTS/LUT_C0_sqrt_1_2.vhd \
	$(SFU_DIR)/Quadratic_Interpolator/Single_LUTS/LUT_C0_sqrt_2_4.vhd \
	$(SFU_DIR)/Quadratic_Interpolator/Single_LUTS/LUT_C1_cos.vhd \
	$(SFU_DIR)/Quadratic_Interpolator/Single_LUTS/LUT_C1_exp.vhd \
	$(SFU_DIR)/Quadratic_Interpolator/Single_LUTS/LUT_C1_ln2.vhd \
	$(SFU_DIR)/Quadratic_Interpolator/Single_LUTS/LUT_C1_ln2e0.vhd \
	$(SFU_DIR)/Quadratic_Interpolator/Single_LUTS/LUT_C1_reci.vhd \
	$(SFU_DIR)/Quadratic_Interpolator/Single_LUTS/LUT_C1_reci_sqrt_1_2.vhd \
	$(SFU_DIR)/Quadratic_Interpolator/Single_LUTS/LUT_C1_reci_sqrt_2_4.vhd \
	$(SFU_DIR)/Quadratic_Interpolator/Single_LUTS/LUT_C1_sin.vhd \
	$(SFU_DIR)/Quadratic_Interpolator/Single_LUTS/LUT_C1_sqrt_1_2.vhd \
	$(SFU_DIR)/Quadratic_Interpolator/Single_LUTS/LUT_C1_sqrt_2_4.vhd \
	$(SFU_DIR)/Quadratic_Interpolator/Single_LUTS/LUT_C2_cos.vhd \
	$(SFU_DIR)/Quadratic_Interpolator/Single_LUTS/LUT_C2_exp.vhd \
	$(SFU_DIR)/Quadratic_Interpolator/Single_LUTS/LUT_C2_ln2.vhd \
	$(SFU_DIR)/Quadratic_Interpolator/Single_LUTS/LUT_C2_ln2e0.vhd \
	$(SFU_DIR)/Quadratic_Interpolator/Single_LUTS/LUT_C2_reci.vhd \
	$(SFU_DIR)/Quadratic_Interpolator/Single_LUTS/LUT_C2_reci_sqrt_1_2.vhd \
	$(SFU_DIR)/Quadratic_Interpolator/Single_LUTS/LUT_C2_reci_sqrt_2_4.vhd \
	$(SFU_DIR)/Quadratic_Interpolator/Single_LUTS/LUT_C2_sin.vhd \
	$(SFU_DIR)/Quadratic_Interpolator/Single_LUTS/LUT_C2_sqrt_1_2.vhd \
	$(SFU_DIR)/Quadratic_Interpolator/Single_LUTS/LUT_C2_sqrt_2_4.vhd \
	$(SFU_DIR)/Quadratic_Interpolator/squaring.vhd \
	$(SFU_DIR)/Quadratic_Interpolator/ROM.vhd \
	$(SFU_DIR)/Quadratic_Interpolator/Quadratic_Interpolator.vhd \
	$(SFU_DIR)/sfu.vhd \
	$(SFU_DIR)/sfu_tb.vhd

.PHONY: all data analyze elaborate run verify clean

all: verify

$(WORKDIR):
	mkdir -p $@

data:
	@echo "Generating SFU input data: N_DATA=$(N_DATA)"
	$(PYTHON) generate_input_data.py --output-dir $(DATA_DIR) --n-data $(N_DATA)

analyze: $(WORKDIR)
	@echo "Analyzing SFU_13 VHDL sources with GHDL..."
	$(GHDL) -a $(GHDLFLAGS) --workdir=$(WORKDIR) $(VHDL_SOURCES)

elaborate: analyze
	@echo "Elaborating sfu_tb..."
	$(GHDL) -e $(GHDLFLAGS) --workdir=$(WORKDIR) sfu_tb

run: data elaborate
	@echo "Running GHDL simulation: sfu_tb STOP_TIME=$(STOP_TIME), N_DATA=$(N_DATA)"
	$(GHDL) -r $(GHDLFLAGS) --workdir=$(WORKDIR) sfu_tb --stop-time=$(STOP_TIME)

verify: run
	@echo "Analyzing GHDL simulation results..."
	$(PYTHON) analyze_results.py --data-dir $(DATA_DIR) --n-data $(N_DATA)

clean:
	rm -rf $(WORKDIR) $(DATA_DIR) __pycache__
