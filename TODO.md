# TODO

## 实现 C model

1. 分析 Golden-model
1. 理解文档:
   - docs/QuadraticIEEETC0305.pdf
   - docs/Implementación y evaluación de una unidad de funciones especiales tolerante a fallas basada en aproximación polinomial por partes.pdf
1. 使用Octave 生成的lut构建SFU硬件算法
1. 算法在cmodel/src 实现

### 测试

1. 在 cmodel/tests 中实现测试，生成大量数据，注入测试
1. 测试数据使用txt格式结构, 文件可以如下
    - cmodel/build/input_data/reci.txt
    - cmodel/build/input_data/sqrt_1_2.txt
    - cmodel/build/input_data/sqrt_2_4.txt
    - cmodel/build/input_data/exp.txt
1. 对比 Octave 输入tests中的数据, 对比cmodel与octave的精度

## 注意

1. 小步修改频繁回归测试
2. 结束需要 review，禁止擅自 git commit
3. 禁止修改代码格式，仅修改项目需要的
