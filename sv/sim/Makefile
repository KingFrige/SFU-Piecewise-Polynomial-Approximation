VERILATOR ?= verilator
PYTHON ?= python3
OCTAVE ?= octave
N_DATA ?= 16

BUILD_DIR := build
DATA_DIR := $(BUILD_DIR)/SFU_Input_data
LUT_DIR := $(BUILD_DIR)/octave_luts
RTL_DIR := ../SFU
BUILD_STAMP := $(BUILD_DIR)/.dir
C_MODEL := ../../cmodel/build/sfu
VHDL_DIR := ../../Description/SFU_13
VHDL_SFU_DIR := $(VHDL_DIR)/SFU
VHDL_BUILD_DIR := $(BUILD_DIR)/vhdl_reference
VHDL_DATA_DIR := $(VHDL_BUILD_DIR)/SFU_Input_data
VHDL_WORK_DIR := $(VHDL_BUILD_DIR)/work

VHDL_SOURCES := \
	$(VHDL_SFU_DIR)/RRO/Components/fp_leading_zeros_and_shift.vhd \
	$(VHDL_SFU_DIR)/RRO/Components/right_shifter.vhd \
	$(VHDL_SFU_DIR)/RRO/Components/add_sub.vhd \
	$(VHDL_SFU_DIR)/RRO/Components/multFP.vhd \
	$(VHDL_SFU_DIR)/RRO/RRO_trig.vhd \
	$(VHDL_SFU_DIR)/RRO/rro.vhd \
	$(VHDL_SFU_DIR)/Components/CLZ.vhd \
	$(VHDL_SFU_DIR)/Components/SFU_Exceptions.vhd \
	$(VHDL_SFU_DIR)/fused_accm_tree/Booth_PP.vhd \
	$(VHDL_SFU_DIR)/fused_accm_tree/CSA_4_2.vhd \
	$(VHDL_SFU_DIR)/fused_accm_tree/fused_accum_tree.vhd \
	$(VHDL_SFU_DIR)/voter/voter.vhd \
	$(wildcard $(VHDL_SFU_DIR)/Quadratic_Interpolator/Single_LUTS/*.vhd) \
	$(VHDL_SFU_DIR)/Quadratic_Interpolator/squaring.vhd \
	$(VHDL_SFU_DIR)/Quadratic_Interpolator/ROM.vhd \
	$(VHDL_SFU_DIR)/Quadratic_Interpolator/Quadratic_Interpolator.vhd \
	$(VHDL_SFU_DIR)/sfu.vhd \
	$(VHDL_SFU_DIR)/sfu_tb.vhd

RTL_SOURCES := $(addprefix $(RTL_DIR)/, \
	sfu_pkg.sv \
	voter.sv \
	squaring.sv \
	booth_pp.sv \
	csa_4_2.sv \
	fused_accum_tree.sv \
	sfu_rom.sv \
	rro_fp.sv \
	rro.sv \
	rro_trig.sv \
	clz.sv \
	sfu_exceptions.sv \
	quadratic_interpolator.sv \
	sfu.sv)

.PHONY: all data luts build run verify smoke voter cmodel-ref compare-cmodel vhdl-reference compare lint synth-check clean

all: verify

$(BUILD_STAMP):
	mkdir -p $(BUILD_DIR)
	touch $@

data:
	$(PYTHON) generate_input_data.py --output-dir $(DATA_DIR) --n-data $(N_DATA)

luts: | $(BUILD_STAMP)
	$(MAKE) -C ../../Octave generate-luts LUT_DIR=$(abspath $(LUT_DIR))

build: luts | $(BUILD_STAMP)
	$(VERILATOR) --binary --timing --top-module tb_sfu -Mdir $(BUILD_DIR)/obj_sfu $(RTL_SOURCES) tb_sfu.sv

run: data build
	$(BUILD_DIR)/obj_sfu/Vtb_sfu +DATA_DIR=$(abspath $(DATA_DIR))

verify: run
	$(PYTHON) analyze_results.py --data-dir $(DATA_DIR) --n-data $(N_DATA)

smoke:
	$(MAKE) verify N_DATA=1

voter:
	mkdir -p $(BUILD_DIR)/obj_voter
	$(VERILATOR) --binary --timing --top-module tb_voter -Mdir $(BUILD_DIR)/obj_voter $(RTL_DIR)/voter.sv tb_voter.sv
	$(BUILD_DIR)/obj_voter/Vtb_voter

cmodel-ref:
	$(MAKE) -C ../../cmodel all

compare-cmodel: verify cmodel-ref
	$(PYTHON) compare_reference.py --data-dir $(DATA_DIR) --result-dir $(BUILD_DIR)/compare_reference --sfu $(abspath $(C_MODEL)) --lut-dir $(abspath $(LUT_DIR))

vhdl-reference: data
	rm -rf $(VHDL_BUILD_DIR)
	mkdir -p $(VHDL_WORK_DIR) $(VHDL_DATA_DIR)
	cp $(DATA_DIR)/input_*.csv $(VHDL_DATA_DIR)/
	ghdl -a --std=93c -fsynopsys --workdir=$(VHDL_WORK_DIR) $(VHDL_SOURCES)
	ghdl -e --std=93c -fsynopsys --workdir=$(VHDL_WORK_DIR) sfu_tb
	cd $(VHDL_BUILD_DIR) && ghdl -r --std=93c -fsynopsys --workdir=$(abspath $(VHDL_WORK_DIR)) sfu_tb --stop-time=20ms

compare: run vhdl-reference
	$(PYTHON) compare_vhdl.py --sv-dir $(DATA_DIR) --vhdl-dir $(VHDL_DATA_DIR) --result-dir $(BUILD_DIR)/compare_vhdl

lint: luts
	$(VERILATOR) --lint-only --timing --top-module tb_sfu $(RTL_SOURCES) tb_sfu.sv

synth-check:
	@if rg -n '\breal\b|shortreal|\$$floor|\$$ceil|\$$ln|\$$pow|\$$value\$$plusargs' $(RTL_DIR); then \
		echo "non-synthesizable behavioral construct found under $(RTL_DIR)"; \
		exit 1; \
	fi
	$(VERILATOR) --lint-only --top-module sfu $(RTL_SOURCES)
	$(VERILATOR) --lint-only --top-module rro $(RTL_SOURCES)

clean:
	rm -rf $(BUILD_DIR) __pycache__
