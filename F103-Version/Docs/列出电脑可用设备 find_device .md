📌 功能
列出电脑上所有可用的串口设备（COM口），用于确认蓝牙模块的连接状态。

🔧 依赖
pyserial

🚀 使用方法
bash
python list_ports.py
📊 输出示例
text
可用串口设备：
COM3 - USB Serial Port (COM3)
COM5 - Bluetooth Serial Port (COM5)  ← 你的JDY-31
COM7 - Arduino Uno (COM7)
🕳️ 踩坑记录
坑1：蓝牙配对后不显示COM口
现象：JDY-31已配对，但设备管理器里没有出现COM口
原因：Windows默认不会为蓝牙SPP设备自动创建COM口
解决：

打开「控制面板」→「设备和打印机」

找到你的蓝牙模块，右键「属性」

「硬件」→ 选中「Standard Serial over Bluetooth link」

「属性」→「端口设置」→「高级」

在「COM端口号」下拉菜单中指定一个未使用的COM口（如COM8）

勾选「传出」

坑2：串口被占用
现象：运行脚本时提示 PermissionError: Access is denied
原因：COM口被其他程序（如串口助手、Arduino IDE）占用了
解决：关闭所有可能占用该COM口的程序

坑3：蓝牙模块不响应
现象：列表里有COM口，但发AT指令没反应
原因：波特率不匹配（JDY-31默认9600）
解决：

python
# 尝试用9600波特率连接
ser = serial.Serial('COM8', 9600, timeout=1)
ser.write(b'AT\r\n')
print(ser.readline())  # 应该返回 OK