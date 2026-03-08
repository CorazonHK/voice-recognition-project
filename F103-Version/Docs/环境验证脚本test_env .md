📌 功能
验证Python环境是否正确配置，检查所有依赖库是否安装成功。

🔧 依赖
tensorflow

numpy

librosa

matplotlib

pyserial

🚀 使用方法
bash
python test_env.py
📊 预期输出示例
text
========================================
Python环境验证
========================================
Python版本: 3.11.9
TensorFlow版本: 2.15.0
NumPy版本: 1.24.3
librosa版本: 0.10.1
matplotlib版本: 3.7.2
pyserial已安装: ✓

测试TensorFlow基础功能...
矩阵乘法测试: 
[[1 3]
 [3 7]]

✅ 环境配置成功！
🕳️ 踩坑记录
坑1：TensorFlow安装失败
现象：pip install tensorflow 报错找不到匹配版本
原因：Python 3.12/3.13 与 TensorFlow 不兼容
解决：

bash
# 安装 Python 3.11，然后用：
pip install tensorflow==2.15.0
坑2：librosa安装卡住
现象：pip install librosa 卡在 building wheel
原因：llvmlite依赖问题
解决：

bash
# 先装llvmlite，再装librosa
pip install llvmlite==0.40.0
pip install librosa
坑3：虚拟环境激活报错
现象：.venv\Scripts\activate 提示“无法加载文件...禁止运行脚本”
原因：PowerShell执行策略限制
解决：

powershell
# 以管理员身份运行PowerShell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser