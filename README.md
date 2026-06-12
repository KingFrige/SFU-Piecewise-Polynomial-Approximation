# SFU – Special Function Unit (Piecewise Polynomial Approximation)

The **Special Function Unit (SFU)** is a domain-specific hardware accelerator for high-performance computation of transcendental and trigonometric functions. This module is commonly integrated into GPUs to support rasterization and compute operations.

This open-source SFU leverages **Piecewise Polynomial Approximation** to achieve:

* 🚀 **High performance**
* 📦 **Low area cost**
* 🌟 **Good accuracy** for real hardware implementations

The current version was evaluated and integrated into the [FlexGripPlus](https://github.com/Jerc007/Open-GPGPU-FlexGrip-) GPGPU model, but can also be used as:

* A **standalone accelerator**
* A **coprocessor** in processor-based systems

---

## 📖 Table of Contents

* [Features](#-features)
* [Supported Functions](#-supported-functions)
* [Golden Model & LUT Generation](#-golden-model--lut-generation)
* [Validation & Synthesis](#-validation--synthesis)
* [Getting Started](#-getting-started)
* [Integration](#-integration)
* [License](#-license)
* [Citation](#-citation)
* [Authors](#-authors)

---

## ✨ Features

* Supports **IEEE-754 floating-point** inputs
* Fused design to reduce area and power consumption
* Verified in simulation and synthesized for ASIC and FPGA targets
* Open-source and customizable

---

## 📐 Supported Functions

The SFU currently implements the following mathematical operations:

| Function  | Description            |
| --------- | ---------------------- |
| `sin(x)`  | Sine                   |
| `cos(x)`  | Cosine                 |
| `log₂(x)` | Binary logarithm       |
| `1/x`     | Reciprocal             |
| `√x`      | Square root            |
| `1/√x`    | Reciprocal square root |
| `2^x`     | Power of two           |

---

## 🧮 Golden Model & LUT Generation

The Octave wrapper is a bit-oriented reference for the hardware datapath, not a high-precision math library. It reuses shared helpers from `Golden-model/` and converts an IEEE-754 hex input into the same range-reduced and LUT-driven approximation path used by the SFU.

```text
IEEE-754 hex input
    │
    ├─ hex / binary parsing
    ▼
range reduction and function remap
    │
    ├─ exp2: exponent/fraction transform
    ├─ sin/cos: quadrant reduction
    └─ sqrt/rsqrt/log2: LUT variant selection
    ▼
loadLUTs(func)
    │
    ├─ coeff(): C0/C1/C2 coefficients
    ├─ coeffbin(): binary fixed-point strings
    └─ getLUT(): common-bit compression for LUT buses
    ▼
F = C0 + C1*x + C2*x^2
    ▼
exponent/sign correction
    ▼
IEEE-754 hex result
```

Run a single evaluation from the migrated Octave directory:

```sh
cd Octave
octave --no-gui --no-window-system --quiet run_sfu_golden_model.m C23A36C1 10
```

Generate LUT matrices in Octave with `loadLUTs(func)`. The function returns in-memory bit matrices; it does not rewrite the VHDL LUT files.

```sh
cd Octave
octave --no-gui --no-window-system --quiet --eval \
  "[LUTC0,LUTC1,LUTC2,m]=loadLUTs(9); \
   printf('m=%d rows=%d widths=%d/%d/%d\n', m, rows(LUTC0), columns(LUTC0), columns(LUTC1), columns(LUTC2)); \
   printf('C0[1]=%s\n', LUTC0(1,:));"
```

Function selectors are `1=1/x`, `2=sqrt`, `4=1/sqrt`, `6=2^x`, `7=log2`, `9=sin`, and `10=cos`. Selectors `3`, `5`, and `8` are internal LUT variants chosen by the model.

To write complete LUT tables to files, use:

```sh
cd Octave
octave --no-gui --no-window-system --quiet run_generate_lut_tables.m generated_luts
```

Pass selector numbers to generate only specific SFU tables:

```sh
octave --no-gui --no-window-system --quiet run_generate_lut_tables.m generated_luts 1 2 9 10
```

Each selector directory contains `LUTC0.txt`, `LUTC1.txt`, `LUTC2.txt`, and `metadata.txt`. The generated text rows are complete hardware bus values: 29 bits for C0, 20 bits for C1, and 14 bits for C2.

The `lut/` directory contains a standalone C implementation of coefficient generation, fixed-point encoding, and LUT file emission:

```sh
cd lut
make test
./build/coeffgen generated_coeffs
```

The C generator mirrors the `Octave/generated_luts` directory layout and binary row format: one directory per function, such as `generated_coeffs/06_exp/`, containing `LUTC0.txt`, `LUTC1.txt`, `LUTC2.txt`, and `metadata.txt`.

---

## 🛠 Validation & Synthesis

The SFU has been validated in:

* **Simulation environments**

  * Octave (as a golden reference model)
  * ModelSim-Altera Starter Edition
  * QuestaSim
    
* **Hardware synthesis**

  * Target platform: **DE2-115 FPGA** (Intel-Altera)
  * 45nm OpenCell library
  * 15nm OpenCell library

---

## 🚀 Getting Started

### Prerequisites

Install the tools required for the flow you want to run:

* GNU Make and a C11 compiler such as GCC or Clang for `lut/` and `cmodel/`
* Python 3 for generated-data and LUT comparison helper scripts
* GNU Octave for the golden model, LUT generation, and C model comparisons
* `pkg-config`, MPFR, and GMP development libraries for the optional MPFR LUT generator variant
* ModelSim-Altera or QuestaSim for HDL simulation
* Quartus Prime for FPGA synthesis

On Debian/Ubuntu systems, the software dependencies for the Octave, LUT, and C model flows can be installed with:

```sh
sudo apt-get install build-essential python3 octave pkg-config libmpfr-dev libgmp-dev
```

The HDL simulator and Quartus Prime are vendor tools and must be installed separately.

### Synthesis

Open the project in Quartus Prime and compile for the target FPGA.

---

## 🔗 Integration

The SFU can be integrated as:

* A dedicated **unit in GPU architectures** (e.g., FlexGripPlus)
* A **coprocessor** in general-purpose processor systems
* A **standalone accelerator** for custom hardware applications

Refer to the [integration guide](docs/integration.md) (coming soon) for detailed instructions.

---

## 📜 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

---

# 📖 Citation

If you use this repository as part of your research, please cite the following paper:

Rodriguez Condia, J.E., Guerrero-Balaguera, JD., Patiño Núñez, E.J. et al. Investigating and Reducing the Architectural Impact of Transient Faults in Special Function Units for GPUs. J Electron Test 40, 215–228 (2024). https://doi.org/10.1007/s10836-024-06107-9

```bibtex
@article{SFU_2024,
  author    = {Rodriguez Condia, Josie E. and Guerrero-Balaguera, Juan-David and Patiño Núñez, Edwar J. and Limas, Robert and Sonza Reorda, Matteo},
  title     = {Investigating and Reducing the Architectural Impact of Transient Faults in Special Function Units for GPUs},
  journal   = {Journal of Electronic Testing},
  year      = {2024},
  volume    = {40},
  pages     = {215–228},
  doi       = {10.1007/s10836-024-06107-9},
  url       = {https://doi.org/10.1007/s10836-024-06107-9}
}
```

---

## 👥 Authors

Developed by the **Robotics and Industrial Automation (GIRA)** Research Group at *Universidad Pedagógica y Tecnológica de Colombia (UPTC)*, in collaboration with the **CAD Group**, *Politecnico di Torino, Italy*.

* **Edwar Javier Patiño Núñez** – [edward.patino@uptc.edu.co](mailto:edward.patino@uptc.edu.co)
* **Juan David Guerrero Balaguera** – [juandavid.guerrero@polito.it](mailto:juandavid.guerrero@polito.it)
* **Josie Esteban Rodriguez Condia** – [josie.rodriguez@polito.it](mailto:josie.rodriguez@polito.it)

---

Complementary material:

* J. E. R. Condia, J. -D. Guerrero-Balaguera, E. J. Patiño Núñez, R. Limas, and M. Sonza Reorda, "[Evaluating the Prevalence of SFUs in the Reliability of GPUs](https://ieeexplore.ieee.org/document/10174110)," 2023 IEEE European Test Symposium (ETS), 2023

* J. E. R. Condia, J. -D. Guerrero-Balaguera, E. J. Patiño Núñez, R. Limas, and M. Sonza Reorda, "[Analyzing the Architectural Impact of Transient Fault Effects in SFUs of GPUs](https://ieeexplore.ieee.org/document/10154504)," 2023 IEEE 24th Latin American Test Symposium (LATS), 2023, pp. 1-6.

* J. E. R. Condia, J. -D. Guerrero-Balaguera, C. -F. Moreno-Manrique, and M. Sonza Reorda, "[Design and Verification of an open-source SFU model for GPGPUs](https://ieeexplore.ieee.org/document/9276748)," in 17th Biennial Baltic Electronics Conference (BEC), 2020

* S. F. Oberman and M. Y. Siu, "[A high-performance area-efficient multifunction interpolator](https://ieeexplore.ieee.org/document/1467649)," in 17th IEEE Symposium on Computer Arithmetic (ARITH'05), pp. 272-279, doi: 10.1109/ARITH.2005.7.

* Michael J. Flynn and Stuart F. Oberman, [Advanced Computer Arithmetic Design](https://www.amazon.it/Advanced-Computer-Arithmetic-Design-English-ebook/dp/B00134C1E0), 1° Edition, 2008.
