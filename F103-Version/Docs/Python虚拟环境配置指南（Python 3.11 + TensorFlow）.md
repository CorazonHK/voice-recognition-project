## Python 虚拟环境配置指南（Python 3.11 + TensorFlow）

本文档记录本项目PC端的Python环境配置过程，基于Python 3.11 + TensorFlow 2.15.0，包含虚拟环境创建、国内源配置、依赖安装等完整步骤。

---

### 📋 环境概述

| 组件       | 版本       | 说明                         |
| ---------- | ---------- | ---------------------------- |
| Python     | 3.11.x     | TensorFlow官方推荐的稳定版本 |
| TensorFlow | 2.15.0     | 与Python 3.11兼容的最新版本  |
| 操作系统   | Windows 11 | 本文以Windows为例            |
| 包管理     | pip        | 配合国内镜像加速             |

---

### 🔧 第一步：安装Python 3.11

#### 1.1 下载安装包

访问 Python 官方下载页面：
[https://www.python.org/downloads/windows/](https://www.python.org/downloads/windows/)

找到 **Python 3.11.x** 版本（例如 3.11.9），下载 **Windows installer (64-bit)**。

#### 1.2 安装步骤

1. 双击下载的安装包
2. **关键步骤**：勾选 ✅ **Add Python 3.11 to PATH**
3. 点击 **Install Now**
4. 等待安装完成

#### 1.3 验证安装

打开命令提示符（cmd）或 PowerShell，输入：

```bash
python --version
```

预期输出：
```
Python 3.11.x
```

---

### 📁 第二步：创建项目文件夹

在你喜欢的位置创建项目根目录（例如 `E:\Tool\codes\voice_recognition`）：

```bash
# 进入目标目录（示例）
cd E:\Tool\codes

# 创建项目文件夹
mkdir voice_recognition

# 进入项目文件夹
cd voice_recognition
```

---

### 🚀 第三步：创建虚拟环境

#### 3.1 创建环境

在项目根目录下执行：

```bash
python -m venv .venv
```

这会在当前目录生成一个 `.venv` 文件夹，里面包含独立的Python解释器和包管理工具。

#### 3.2 激活虚拟环境

**Windows PowerShell：**

```bash
.venv\Scripts\activate
```

**Windows CMD：**

```bash
.venv\Scripts\activate.bat
```

**Mac/Linux：**

```bash
source .venv/bin/activate
```

**成功标志：** 命令行前出现 `(.venv)` 字样

```
(.venv) E:\Tool\codes\voice_recognition>
```

---

### ⚙️ 第四步：配置pip国内源（加速下载）

#### 4.1 方法一：临时使用（单次生效）

```bash
pip install -i https://pypi.tuna.tsinghua.edu.cn/simple 包名
```

#### 4.2 方法二：永久配置（推荐）

在Windows用户目录下创建pip配置文件：

1. 打开文件资源管理器，地址栏输入 `%APPDATA%` 并回车
2. 进入 `pip` 文件夹（如不存在则新建）
3. 创建 `pip.ini` 文件，用记事本打开，写入：

```ini
[global]
index-url = https://pypi.tuna.tsinghua.edu.cn/simple
[install]
trusted-host = pypi.tuna.tsinghua.edu.cn
```

保存后，后续所有 `pip install` 都会走清华源。

#### 4.3 验证配置

```bash
pip config list
```

如果显示 `index-url='https://pypi.tuna.tsinghua.edu.cn/simple'`，说明配置成功。

---

### 📦 第五步：安装核心依赖

#### 5.1 升级pip

```bash
python -m pip install --upgrade pip
```

#### 5.2 安装TensorFlow

```bash
pip install tensorflow==2.15.0
```

**说明**：TensorFlow 2.15.0 是最后一个官方支持Python 3.11的版本，2.16+需要Python 3.12。

#### 5.3 安装其他必需库

```bash
pip install numpy          # 数组操作
pip install scipy          # 科学计算
pip install librosa        # 音频特征提取（MFCC）
pip install matplotlib     # 绘图（可视化波形、训练曲线）
pip install pyserial       # 串口通信（接收STM32数据）
```

#### 5.4 一键安装（可选）

将以上依赖写入 `requirements.txt`：

```txt
tensorflow==2.15.0
numpy
scipy
librosa
matplotlib
pyserial
```

然后执行：

```bash
pip install -r requirements.txt
```

---

### ✅ 第六步：验证安装

创建测试文件 `test_env.py`：

```python
"""
环境验证脚本
运行该脚本检查所有关键库是否正确安装
"""

import sys
import tensorflow as tf
import numpy as np
import librosa
import matplotlib
import serial

print("=" * 40)
print("Python环境验证")
print("=" * 40)

print(f"Python版本: {sys.version}")
print(f"TensorFlow版本: {tf.__version__}")
print(f"NumPy版本: {np.__version__}")
print(f"librosa版本: {librosa.__version__}")
print(f"matplotlib版本: {matplotlib.__version__}")
print(f"pyserial已安装: {'✓' if serial else '✗'}")

# 测试TensorFlow基本功能
print("\n测试TensorFlow基础功能...")
a = tf.constant([[1, 2], [3, 4]])
b = tf.constant([[1, 1], [0, 1]])
c = tf.matmul(a, b)
print(f"矩阵乘法测试: \n{c.numpy()}")

print("\n✅ 环境配置成功！")
```

运行测试：

```bash
python test_env.py
```

**预期输出**应包含所有库的版本号，且无报错信息。

---

### 📝 第七步：记录依赖（可选）

将当前环境的所有依赖导出，方便后续在其他电脑上复现：

```bash
pip freeze > requirements-full.txt
```

---

### ❓ 常见问题排查

#### Q1：`pip` 命令找不到

```bash
# 用 python -m pip 代替
python -m pip install 包名
```

#### Q2：TensorFlow安装失败

```bash
# 尝试指定版本
pip install tensorflow-cpu==2.15.0
```

#### Q3：激活虚拟环境时报错“无法加载文件...因为在此系统上禁止运行脚本”

**解决方法**（以管理员身份运行PowerShell）：

```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

然后重新打开终端激活。

#### Q4：安装速度慢

确保已配置国内镜像源（见第四步），或临时使用：

```bash
pip install -i https://pypi.tuna.tsinghua.edu.cn/simple tensorflow==2.15.0
```

---

### 🎯 最终环境状态

完成上述步骤后，你的项目文件夹结构应为：

```
voice_recognition/
├── .venv/                    # 虚拟环境（不要手动修改）
├── test_env.py               # 验证脚本
├── requirements.txt          # 依赖清单（可选）
└── （后续添加的代码文件）
```

每次开始工作前，记得激活虚拟环境：

```bash
.venv\Scripts\activate
```

看到 `(.venv)` 提示符后，即可正常运行所有Python脚本。