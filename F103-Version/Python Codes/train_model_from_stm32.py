"""
train_stm32_model.py - 用STM32采集的MFCC数据训练模型
"""

import numpy as np
import tensorflow as tf
from tensorflow.keras import layers, models
import os
import glob
from sklearn.model_selection import train_test_split

# ========== 配置 ==========
COMMANDS = ['left', 'right', 'stop', 'forward']
DATASET_PATH = 'dataset_stm32'     # 从STM32采集的数据
EPOCHS = 80
BATCH_SIZE = 16
CONTEXT_FRAMES = 11
N_MFCC = 13
# =========================

def load_dataset(data_path):
    """加载.npy格式的MFCC数据"""
    X = []
    y = []
    
    print("加载数据集...")
    
    for label_idx, command in enumerate(COMMANDS):
        pattern = os.path.join(data_path, command, '*.npy')
        files = glob.glob(pattern)
        print(f"  {command}: {len(files)} 个文件")
        
        for file in files:
            try:
                mfcc = np.load(file)  # 形状 (13,)
                
                # 模拟上下文窗口（单帧扩展）
                half = CONTEXT_FRAMES // 2
                window = np.zeros((CONTEXT_FRAMES, N_MFCC))
                window[half] = mfcc
                for j in range(1, half+1):
                    if half - j >= 0:
                        window[half - j] = mfcc * 0.9
                    if half + j < CONTEXT_FRAMES:
                        window[half + j] = mfcc * 0.9
                
                X.append(window)
                y.append(label_idx)
                
            except Exception as e:
                print(f"    错误处理 {file}: {e}")
    
    return np.array(X), np.array(y)

def create_model(input_shape, num_classes):
    """创建模型（和之前一样）"""
    model = models.Sequential([
        layers.Input(shape=input_shape),
        layers.Conv1D(64, 3, activation='relu', padding='same'),
        layers.BatchNormalization(),
        layers.MaxPooling1D(2),
        layers.Dropout(0.1),
        layers.Conv1D(128, 3, activation='relu', padding='same'),
        layers.BatchNormalization(),
        layers.GlobalAveragePooling1D(),
        layers.Dense(64, activation='relu'),
        layers.Dropout(0.1),
        layers.Dense(num_classes, activation='softmax')
    ])
    return model

def main():
    print("=" * 60)
    print("用STM32采集的数据训练MFCC模型")
    print("=" * 60)
    
    X, y = load_dataset(DATASET_PATH)
    print(f"\n数据集大小: {X.shape}")
    
    if len(X) == 0:
        print("❌ 没有数据，先用 collect_from_stm32.py 采集")
        return
    
    X = (X - np.mean(X)) / (np.std(X) + 1e-8)
    
    X_train, X_test, y_train, y_test = train_test_split(
        X, y, test_size=0.2, random_state=42, stratify=y
    )
    
    model = create_model((CONTEXT_FRAMES, N_MFCC), len(COMMANDS))
    model.summary()
    
    model.compile(
        optimizer=tf.keras.optimizers.Adam(0.001),
        loss='sparse_categorical_crossentropy',
        metrics=['accuracy']
    )
    
    callbacks = [
        tf.keras.callbacks.EarlyStopping(patience=12, restore_best_weights=True),
        tf.keras.callbacks.ReduceLROnPlateau(factor=0.5, patience=6, verbose=1),
        tf.keras.callbacks.ModelCheckpoint('stm32_model.h5', save_best_only=True)
    ]
    
    history = model.fit(
        X_train, y_train,
        batch_size=BATCH_SIZE,
        epochs=EPOCHS,
        validation_data=(X_test, y_test),
        callbacks=callbacks,
        verbose=1
    )
    
    test_loss, test_acc = model.evaluate(X_test, y_test)
    print(f"\n📊 测试准确率: {test_acc*100:.2f}%")
    model.save(f'stm32_model_{test_acc*100:.0f}percent.h5')

if __name__ == '__main__':
    main()
