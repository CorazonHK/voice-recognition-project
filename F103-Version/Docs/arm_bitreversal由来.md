# 📝 arm_bitreversal_fix.c 的由来

## 🧠 背景

在使用 CMSIS-DSP 库的 FFT 函数时，遇到链接错误：

Error: L6218E: Undefined symbol arm_bitreversal_32 (referred from arm_cfft_f32.o).
Error: L6218E: Undefined symbol arm_bitreversal_16 (referred from arm_cfft_q15.o).


这两个函数是 FFT 必需的位反转（bit-reversal）函数。

---

## 🔍 尝试过的解决路径

1. **在工程里找了 arm_bitreversal2.s**  
   发现 Keil 安装目录下确实有这个汇编文件，但加到工程里后依然报错。

2. **加了汇编编译选项 `--cpreproc`**  
   有人建议加上这个选项让汇编器支持预处理，但试了还是没用。

3. **换成 arm_bitreversal2.c**  
   找了一下，CMSIS 确实有 C 语言版本，但加到工程里依然不行。

4. **最终确认**  
   汇编文件加了、选项改了、C 文件也加了，还是链接不上。说明不是文件缺失的问题，而是这个 CMSIS 版本（5.0.1）的 DSP 库本身有问题——它要求的外部符号和实际提供的符号对不上。

---

## ✅ 最终决定

既然库本身有问题，就绕过库，自己写。

自己手写了一个 `arm_bitreversal_fix.c`，里面用纯 C 实现了 `arm_bitreversal_32` 和 `arm_bitreversal_16` 两个函数。加上去之后，编译通过，FFT 正常工作。

---