📌 功能
实时接收STM32蓝牙发来的音频数据，调用训练好的模型进行推理，打印识别结果。

🔧 依赖
tensorflow

numpy

librosa

pyserial

⚙️ 配置参数
python
BLUETOOTH_PORT = 'COM11'      # 你的蓝牙虚拟COM口号
BAUD_RATE = 9600        # 波特率（和STM32一致）
SAMPLE_RATE = 8000         # 采样率 16kHz
MODEL_PATH = 'speech_cnn.h5'  # 训练好的模型
COMMANDS = ['left', 'right', 'stop', 'forward']  # 命令词
CONFIDENCE_THRESHOLD = 0.7    # 置信度阈值
🚀 使用方法
bash
python GetMfcc.py
📊 运行输出示例S
text
加载模型: speech_cnn.h5
模型加载成功！
串口 COM8 已打开，等待数据...

[1] 识别结果: left (置信度: 0.95)
[2] 识别结果: right (置信度: 0.92)
[3] 识别结果: unknown (置信度: 0.34)
[4] 识别结果: stop (置信度: 0.88)
...
🔄 数据流
text
STM32 (PA0采集音频) 
    → 蓝牙 (JDY-31) 
    → PC (COM8) 
    → GetMfcc.py (预处理+推理) 
    → 控制台打印结果
🕳️ 踩坑记录
坑1：模型加载失败
现象：FileNotFoundError: [Errno 2] No such file or directory: 'speech_cnn.h5'
原因：模型文件不在当前目录，或者路径写错了
解决：

python
import os
# 使用绝对路径
MODEL_PATH = os.path.join(os.path.dirname(__file__), 'models', 'speech_cnn.h5')
# 或者先检查文件是否存在
if not os.path.exists(MODEL_PATH):
    print(f"错误：找不到模型文件 {MODEL_PATH}")
    exit(1)
坑2：音频数据对齐问题
现象：识别结果乱跳，准确率很低
原因：STM32发送的音频数据和训练时的预处理不一致
解决：确保两端参数完全一致

python
# 训练时提取MFCC的参数
mfcc_train = librosa.feature.mfcc(
    y=audio, 
    sr=16000, 
    n_mfcc=13, 
    n_fft=512, 
    hop_length=160, 
    window='hamming'
)

# 推理时也要用完全一样的参数
mfcc_infer = librosa.feature.mfcc(
    y=audio, 
    sr=16000, 
    n_mfcc=13, 
    n_fft=512, 
    hop_length=160, 
    window='hamming'
)
坑3：接收到的数据长度不固定
现象：有时收到32000字节，有时收到16000字节
原因：STM32发送任务和PC接收任务不同步
解决：使用固定帧头同步

python
# 等待帧头
while True:
    byte = ser.read(1)
    if byte == b'\xAA':
        if ser.read(1) == b'\x55':
            # 找到帧头，开始读数据
            data = ser.read(1024)  # 1024字节音频数据
            ser.read(2)  # 跳过帧尾 \r\n
            break
坑4：连续运行时内存泄漏
现象：运行时间越长，电脑越卡，内存占用持续增加
原因：在循环中不断创建新对象，没有释放
解决：

python
# 复用数组和缓冲区
audio_buffer = bytearray(32000)  # 预分配缓冲区
mfcc_features = np.zeros((1, 101, 13))  # 预分配特征数组

while True:
    # 用预先分配的缓冲区接收数据
    ser.readinto(audio_buffer)
    # 直接使用预先分配的数组
    mfcc = librosa.feature.mfcc(...)
    mfcc_features[0] = mfcc.T
    pred = model.predict(mfcc_features, verbose=0)
坑5：置信度阈值设置不当
现象：要么漏识别（明明说了却没反应），要么误识别（没说话却乱输出）
原因：阈值太高漏报，阈值太低误报
解决：

python
# 根据实际测试调整
# 先设为0.5，观察一段时间
# 如果误报多，提高到0.7
# 如果漏报多，降低到0.3
CONFIDENCE_THRESHOLD = 0.7

# 也可以记录历史，动态调整
confidence_history = []
# ... 记录每次识别的置信度
avg_confidence = np.mean(confidence_history[-50:])
# 根据平均置信度调整阈值