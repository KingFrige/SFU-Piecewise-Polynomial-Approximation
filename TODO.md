# TODO

## 实现 verilog / sv 版本 RTL

1. 理解文档:
   - docs/QuadraticIEEETC0305.pdf
   - docs/Implementación y evaluación de una unidad de funciones especiales tolerante a fallas basada en aproximación polinomial por partes.pdf
1. 分析 Description/SFU_13 实现
1. 使用 verilog 实现硬件设计，基本结构
   - sv/SFU       --- verilog/sv 实现 -> 将 Description/SFU_13/SFU 使用sv/v描述
   - sv/sim       --- 验证环境        -> 移植 Description/SFU_13/Verification, 使用sv/v 描述
   - sv/sim/Makefile  --- 仿真构建

1. 要求: 实现的 仿真环境使用 verilator
   - module load openEDA/verilator/v5.046

## 注意

1. 小步修改频繁回归测试
2. 结束需要 review，禁止擅自 git commit
3. 禁止修改代码格式，仅修改项目需要的
