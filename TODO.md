# TODO

## 使用 ghdl 进行仿真 vhd

1. 深入理解分析 Description/SFU_13/
1. 现在的仿真工具 modelsim 没有被按照，需要使用 ghdl 替代
1. 理解 Description/SFU_13/Verification/ 的实现
    - 参考创建 Description/SFU_13/ghdl 目录， 使用 ghdl 来仿真
1. 注意移植现在 Description/SFU_13/Verification/ 的流程

### 测试

1. 在 Description/SFU_13/ghdl 添加 Makefile，执行make能 使用python生成相关数据, 能调用 ghdl 仿真

## 注意

1. 小步修改频繁回归测试
2. 结束需要 review，禁止擅自 git commit
3. 禁止修改代码格式，仅修改项目需要的
