"""
mfcc_receiver.py - MFCC实时接收器（带噪声过滤）
接收STM32蓝牙发来的MFCC特征，用训练好的模型识别
噪声帧（能量过低）直接丢弃，不进入模型
"""

import serial
import threading
import time
import sys
import subprocess
import re
import numpy as np
import tensorflow as tf

# ========== 配置 ==========
PORT = 'COM11'                       # 改成你的蓝牙COM口
BAUDRATE = 9600                      # 和STM32一致
MFCC_SIZE = 13                        # MFCC系数个数
EXPECTED_BYTES = 2 + MFCC_SIZE*2 + 2  # AA 55 + 26字节 + 0D 0A = 30字节
BLUETOOTH_DEVICE_NAME = 'HC-06'       # 你的蓝牙设备名称

# 模型参数（和训练时一致）
CONTEXT_FRAMES = 11                    # 上下文帧数（必须和训练时相同）
MODEL_PATH = 'mfcc_model_83percent.h5 '     # 训练好的模型

# 噪声过滤阈值（需要实测调整）
ENERGY_THRESHOLD = 3000# 用所有系数之和判断，阈值设小一点
CONFIDENCE_THRESHOLD = 0.7              # 置信度阈值
COMMANDS = ['left', 'right', 'stop', 'forward']  # 命令词
# =========================

class MFCCRecognizer:
    def __init__(self, model_path):
        print("🔄 加载模型中...")
        self.model = tf.keras.models.load_model(model_path)
        print("✅ 模型加载成功")
        self.commands = COMMANDS
        
        # 缓冲区，用于拼接上下文帧
        self.frame_buffer = []
        self.half = CONTEXT_FRAMES // 2
        
    def parse_frame(self, data):
        """解析一帧MFCC数据"""
        if len(data) < EXPECTED_BYTES:
            return None
        
        # 检查帧头
        if data[0] != 0xAA or data[1] != 0x55:
            return None
        
        # 解析13个MFCC系数
        mfcc = []
        for i in range(MFCC_SIZE):
            idx = 2 + i*2
            value = (data[idx] << 8) | data[idx+1]
            # 调试用：打印原始值
            if i == 0:
                print(f"  raw[0] = {value}")
            # 反量化（和训练时一致，除以100）
            mfcc.append(value / 100.0)
        
        return np.array(mfcc)
    
    def is_speech(self, mfcc_frame):
        """判断当前帧是否是语音（用所有系数的和）"""
        total_energy = np.sum(np.abs(mfcc_frame))
        return total_energy > ENERGY_THRESHOLD
    
    def add_frame(self, mfcc_frame):
        """添加一帧到缓冲区，并返回是否需要识别"""
        self.frame_buffer.append(mfcc_frame)
        
        # 保持缓冲区长度 = CONTEXT_FRAMES
        if len(self.frame_buffer) > CONTEXT_FRAMES:
            self.frame_buffer.pop(0)
        
        # 只有当缓冲区满了才识别
        return len(self.frame_buffer) == CONTEXT_FRAMES
    
    def predict(self):
        """对当前缓冲区中的帧进行识别（带概率打印）"""
        if len(self.frame_buffer) < CONTEXT_FRAMES:
            return None, 0.0
        
        # 检查缓冲区中心帧是否是语音
        center_idx = self.half
        if not self.is_speech(self.frame_buffer[center_idx]):
            return 'noise', 0.0
        
        # 构建输入 (1, CONTEXT_FRAMES, MFCC_SIZE)
        features = np.array(self.frame_buffer).reshape(1, CONTEXT_FRAMES, MFCC_SIZE)
        
        # 预测
        predictions = self.model.predict(features, verbose=0)[0]
        
        # ⭐ 打印所有类别的概率
        prob_str = "  "
        for i, cmd in enumerate(self.commands):
            prob_str += f"{cmd}: {predictions[i]:.3f}  "
        print(prob_str)
        
        idx = np.argmax(predictions)
        conf = predictions[idx]
        
        if conf > CONFIDENCE_THRESHOLD:
            return self.commands[idx], conf
        return 'unknown', conf

def find_bluetooth_com_port(device_name):
    """通过WMI查询蓝牙设备对应的COM口"""
    try:
        cmd2 = 'powershell "[System.IO.Ports.SerialPort]::getportnames()"'
        result2 = subprocess.run(cmd2, capture_output=True, text=True, shell=True)
        
        com_ports = re.findall(r'COM\d+', result2.stdout)
        if com_ports:
            return com_ports[-1]
    except:
        pass
    return None

def connect_bluetooth_device(device_name):
    """尝试连接蓝牙设备"""
    print(f"🔄 尝试连接蓝牙设备: {device_name}")
    
    try:
        # 尝试打开所有可能的COM口来触发连接
        for i in range(0, 256):
            try:
                test_port = f'COM{i}'
                s = serial.Serial(test_port, BAUDRATE, timeout=0.1)
                s.close()
            except:
                pass
    except:
        pass
    
    time.sleep(5)
    return True

def wait_for_com_port(port_name, timeout=20):
    """等待COM口出现并可用"""
    print(f"等待 {port_name} 就绪...")
    start_time = time.time()
    while time.time() - start_time < timeout:
        try:
            s = serial.Serial(port_name, BAUDRATE, timeout=0.1)
            s.close()
            print(f"✅ {port_name} 已就绪")
            return True
        except:
            time.sleep(2)
    return False

def serial_receiver():
    """串口接收线程"""
    global recognizer
    
    # 先确保端口可用
    if not wait_for_com_port(PORT):
        print(f"❌ {PORT} 不可用，尝试主动连接蓝牙")
        connect_bluetooth_device(BLUETOOTH_DEVICE_NAME)
        
        if not wait_for_com_port(PORT):
            print(f"❌ {PORT} 仍然不可用，请检查蓝牙连接")
            return
    
    try:
        ser = serial.Serial(PORT, BAUDRATE, timeout=1)
        print(f"✅ 打开串口 {PORT} 成功")
        print("等待数据...")
        print(f"能量阈值: {ENERGY_THRESHOLD}, 置信度阈值: {CONFIDENCE_THRESHOLD}")
        print("-" * 60)
        
        buffer = bytearray()
        frame_count = 0
        speech_count = 0
        noise_count = 0
        unknown_count = 0
        
        while True:
            data = ser.read(64)
            if data:
                # 调试信息：打印收到的字节数
                print(f"📥 收到 {len(data)} 字节 | 缓冲区大小: {len(buffer)}")
                
                buffer.extend(data)
                
                while len(buffer) >= EXPECTED_BYTES:
                    # 查找帧头
                    start = -1
                    for i in range(len(buffer) - 1):
                        if buffer[i] == 0xAA and buffer[i+1] == 0x55:
                            start = i
                            break
                    
                    if start == -1 or len(buffer) - start < EXPECTED_BYTES:
                        # 没找到完整帧，保留数据继续接收
                        break
                    
                    # 提取一帧
                    frame_data = buffer[start:start+EXPECTED_BYTES]
                    buffer = buffer[start+EXPECTED_BYTES:]
                    
                    # 调试信息：打印帧头和数据
                    print(f"  帧头: {frame_data[0]:02X} {frame_data[1]:02X}")
                    print(f"  数据长度: {len(frame_data)} 字节")
                    
                    # 解析MFCC
                    mfcc = recognizer.parse_frame(frame_data)
                    if mfcc is not None:
                        # ⭐ 在这里打印MFCC范围
                        print(f"  MFCC范围: min={np.min(mfcc):.2f}, max={np.max(mfcc):.2f}, mean={np.mean(mfcc):.2f}")
                        
                        frame_count += 1
                        
                        # 判断是否是语音
                        if recognizer.is_speech(mfcc):
                            speech_count += 1
                            
                            # 添加到缓冲区，准备识别
                            if recognizer.add_frame(mfcc):
                                command, conf = recognizer.predict()
                                
                                if command == 'noise':
                                    noise_count += 1
                                elif command != 'unknown':
                                    print(f"\n🎤 识别: {command} (置信度: {conf:.2f}) | 帧: {frame_count} | 语音帧: {speech_count}")
                                else:
                                    unknown_count += 1
                        else:
                            noise_count += 1
                            print(f"  ⏺️ 噪声帧 (能量低于阈值)")
                    else:
                        print("  ❌ 解析失败")
    
    except serial.SerialException as e:
        print(f"❌ 串口错误: {e}")
    except KeyboardInterrupt:
        print(f"\n📊 统计: 总帧数 {frame_count}, 语音帧 {speech_count}, 噪声帧 {noise_count}, 未知 {unknown_count}")
    finally:
        if 'ser' in locals():
            ser.close()
            print("串口已关闭")

if __name__ == "__main__":
    print("=" * 60)
    print("🎤 MFCC实时识别器（带噪声过滤）")
    print("=" * 60)
    print(f"目标端口: {PORT} @ {BAUDRATE} bps")
    print(f"蓝牙设备: {BLUETOOTH_DEVICE_NAME}")
    print(f"模型: {MODEL_PATH}")
    print(f"上下文帧: {CONTEXT_FRAMES}")
    print(f"命令词: {COMMANDS}")
    print("-" * 60)
    
    # 初始化识别器
    recognizer = MFCCRecognizer(MODEL_PATH)
    
    # 启动接收线程
    receiver_thread = threading.Thread(target=serial_receiver, daemon=True)
    receiver_thread.start()
    
    try:
        while True:
            time.sleep(1)
    except KeyboardInterrupt:
        print("\n程序退出")