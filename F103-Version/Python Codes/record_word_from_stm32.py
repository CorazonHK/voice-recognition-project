"""
record_word_from_stm32.py - 从STM32接收MFCC数据并保存为数据集
按回车开始录音，再按回车结束，g继续，q退出
"""

import serial
import numpy as np
import os
from datetime import datetime
import time
import msvcrt
from collections import deque

# ========== 配置 ==========
PORT = 'COM11'                       # 你的蓝牙COM口
BAUDRATE = 9600                      # 波特率
MFCC_SIZE = 13                        # MFCC系数个数
EXPECTED_BYTES = 2 + MFCC_SIZE*2 + 2  # AA 55 + 26字节 + 0D 0A = 30字节
COMMANDS = ['left', 'right', 'stop', 'forward']  # 命令词
SAVE_DIR = 'dataset_stm32'            # 保存目录

# 录音参数
ENERGY_THRESHOLD = 500                 # 能量阈值
MIN_FRAMES = 2                         # ⭐ 最少保存2帧（从5降到2）
# =========================

class MFCCCollector:
    def __init__(self):
        self.buffer = []
        self.is_recording = False
        self.current_cmd = 0
        self.frame_count = 0
        self.total_saved = 0
        
    def parse_frame(self, data):
        """解析一帧MFCC数据（直接返回int16，忽略溢出警告）"""
        if len(data) < EXPECTED_BYTES:
            return None
        if data[0] != 0xAA or data[1] != 0x55:
            return None
        
        mfcc = []
        for i in range(MFCC_SIZE):
            idx = 2 + i*2
            value = (data[idx] << 8) | data[idx+1]
            # 直接转int16，忽略溢出警告
            mfcc.append(value.astype(np.int16) if hasattr(value, 'astype') else np.int16(value))
        return np.array(mfcc, dtype=np.int16)
    
    def is_speech(self, mfcc):
        """判断是否是语音"""
        energy = np.sum(np.abs(mfcc.astype(np.int32)))  # 转int32避免溢出
        return energy > ENERGY_THRESHOLD
    
    def add_frame(self, mfcc):
        """添加一帧到缓冲区"""
        energy = np.sum(np.abs(mfcc.astype(np.int32)))
        if self.is_recording and energy > ENERGY_THRESHOLD:
            self.buffer.append(mfcc)
            self.frame_count += 1
            return True
        return False
    
    def save_current(self):
        """保存当前缓冲区所有帧"""
        if len(self.buffer) < MIN_FRAMES:
            print(f"\n⚠️ 帧数太少 ({len(self.buffer)} < {MIN_FRAMES})，不保存")
            self.buffer.clear()
            return False
        
        cmd = COMMANDS[self.current_cmd]
        save_path = os.path.join(SAVE_DIR, cmd)
        os.makedirs(save_path, exist_ok=True)
        
        timestamp = datetime.now().strftime('%Y%m%d_%H%M%S')
        
        # 保存所有帧
        for i, mfcc in enumerate(self.buffer):
            filename = f"{cmd}_{timestamp}_frame{i:02d}.npy"
            filepath = os.path.join(save_path, filename)
            np.save(filepath, mfcc.astype(np.int16))
        
        self.total_saved += 1
        print(f"\n✅ 已保存 {len(self.buffer)} 帧到 {cmd}/ (第{self.total_saved}次)")
        self.buffer.clear()
        return True

def get_key():
    """非阻塞获取按键"""
    if msvcrt.kbhit():
        return msvcrt.getch().decode().lower()
    return None

def main():
    print("=" * 60)
    print("🎤 从STM32采集MFCC数据集（2帧就保存）")
    print("=" * 60)
    print(f"命令词: {COMMANDS}")
    print(f"保存位置: {SAVE_DIR}/")
    print(f"能量阈值: {ENERGY_THRESHOLD}")
    print(f"最少保存帧: {MIN_FRAMES} ⭐ 说一个词就够了")
    print("-" * 50)
    print("操作说明：")
    print("  - 按【回车】开始录音")
    print("  - 再按【回车】结束录音并保存")
    print("  - 按【g】继续录同一个词")
    print("  - 按【1-4】切换命令词")
    print("  - 按【q】退出")
    print("-" * 50)
    
    # 打开串口
    try:
        ser = serial.Serial(PORT, BAUDRATE, timeout=0.1)
        print(f"✅ 打开串口 {PORT} 成功")
    except Exception as e:
        print(f"❌ 打开串口失败: {e}")
        return
    
    collector = MFCCCollector()
    
    try:
        while True:
            # 显示当前状态
            cmd = COMMANDS[collector.current_cmd]
            status = "🎤 录音中..." if collector.is_recording else "⏸️ 待机"
            saved_info = f" | 已保存 {collector.total_saved} 次" if collector.total_saved > 0 else ""
            
            # 显示当前能量（如果有最新帧）
            energy_display = ""
            if len(collector.buffer) > 0:
                last_energy = np.sum(np.abs(collector.buffer[-1].astype(np.int32)))
                energy_display = f" | 能量:{last_energy:.0f}"
            
            print(f"\r▶️ {cmd} | {status} | 已收 {len(collector.buffer)} 帧{energy_display}{saved_info}", end='', flush=True)
            
            # 检查按键
            key = get_key()
            if key:
                if key == 'q':
                    print("\n退出")
                    break
                elif key == 'g' and not collector.is_recording:
                    print(f"\n🔄 继续录制 {cmd}")
                elif key in ['1', '2', '3', '4'] and not collector.is_recording:
                    collector.current_cmd = int(key) - 1
                    collector.buffer.clear()
                    print(f"\n切换到: {COMMANDS[collector.current_cmd]}")
                elif key == '\r':  # 回车
                    if not collector.is_recording:
                        # 开始录音
                        collector.is_recording = True
                        collector.buffer.clear()
                        print(f"\n🎤 开始录制 {cmd}... 请说话")
                    else:
                        # 结束录音
                        collector.is_recording = False
                        collector.save_current()
            
            # 读取串口数据
            data = ser.read(64)
            if data:
                buffer = data
                
                while len(buffer) >= EXPECTED_BYTES:
                    # 找帧头
                    start = -1
                    for i in range(len(buffer) - 1):
                        if buffer[i] == 0xAA and buffer[i+1] == 0x55:
                            start = i
                            break
                    
                    if start == -1 or len(buffer) - start < EXPECTED_BYTES:
                        break
                    
                    frame = buffer[start:start+EXPECTED_BYTES]
                    buffer = buffer[start+EXPECTED_BYTES:]
                    
                    mfcc = collector.parse_frame(frame)
                    if mfcc is not None:
                        collector.add_frame(mfcc)
            
            time.sleep(0.01)
            
    except KeyboardInterrupt:
        print("\n退出")
    finally:
        ser.close()

if __name__ == '__main__':
    main()

    