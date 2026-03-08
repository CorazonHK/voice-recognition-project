import tensorflow as tf
import numpy as np
import librosa
import serial

print("TensorFlow 版本:", tf.__version__)
print("NumPy 版本:", np.__version__)
print("librosa 版本:", librosa.__version__)

# 测试 TensorFlow 基本功能
print("\n可用设备:", tf.config.list_physical_devices())