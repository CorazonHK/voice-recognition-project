📌 功能
通过蓝牙接收STM32发来的音频数据，保存为WAV文件。Python主动发送 start/stop 指令控制STM32录音启停。

🔧 依赖
pyserial

wave（Python标准库）

time（Python标准库）

⚙️ 配置参数
python
BLUETOOTH_PORT = 'COM11'      # 你的蓝牙虚拟COM口号
BAUD_RATE = 9600           # 波特率（和STM32一致）
SAMPLE_RATE = 8000         # 采样率 8kHz
CHANNELS = 1                  # 单声道
SAMPLE_WIDTH = 2              # 16bit = 2字节
RECORD_SECONDS = 2            # 每条语音时长（秒）
🚀 使用方法
bash
python record_data.py
按提示依次录制 left、right、stop、forward 等命令词。

📁 输出结构
text
dataset/
├── left/
│   ├── left_20260222_143001.wav
│   └── left_20260222_143005.wav
├── right/
│   ├── right_20260222_143101.wav
│   └── ...
├── stop/
└── forward/
🔄 通信协议
方向	指令	说明
PC → STM32	"start\n"	开始录音
PC → STM32	"stop\n"	停止录音
STM32 → PC	AA 55 [1024字节数据] 0D 0A	音频数据帧
🕳️ 踩坑记录
坑1：录下来的WAV文件播放没声音
现象：能录到文件，但播放时全是噪音或静音
原因：字节序错误（小端/大端搞反）
解决：

python
# 错误写法（直接写入）
wav.writeframesraw(data)

# 正确写法（需要转换字节序）
import numpy as np
# 假设STM32发的是小端有符号16位
samples = np.frombuffer(data, dtype='<i2')
wav.writeframesraw(samples.tobytes())
坑2：录音卡顿/丢数据
现象：录到的声音断断续续，时长对不上
原因：PC接收速度跟不上STM32发送速度
解决：

python
# 增加串口缓冲区
ser = serial.Serial('COM8', 115200, timeout=1)
ser.set_buffer_size(rx_size=65536)  # 64KB缓冲区

# 或者降低采样率（STM32端改）
# 16kHz → 8kHz
坑3：发送start后STM32没反应
现象：发完start，串口收不到数据
原因：换行符问题（STM32可能只认 \n 或只认 \r\n）
解决：

python
# 两种都试试
ser.write(b'start\n')
# 或者
ser.write(b'start\r\n')
坑4：录到一半卡死
现象：运行一段时间后脚本无响应
原因：没有处理串口超时
解决：

python
ser.timeout = 0.5  # 设置超时时间
try:
    data = ser.read(size)
except serial.SerialTimeoutException:
    print("超时，重试...")
    continue