📌 功能
用自己录制的语音数据训练CNN模型，实现命令词识别。

🔧 依赖
tensorflow

numpy

librosa

matplotlib

os（标准库）

⚙️ 配置参数
python
SAMPLE_RATE = 8000        # 采样率
DURATION = 1.0                # 每条语音时长（秒）
N_MFCC = 13                   # MFCC特征数
COMMANDS = ['left', 'right', 'stop', 'forward']  # 你的命令词
DATASET_PATH = 'dataset'       # 数据集路径
EPOCHS = 30                    # 训练轮数
BATCH_SIZE = 32                # 批次大小
🚀 使用方法
bash
python train_model.py
📊 训练过程输出
text
正在加载数据集...
  加载 left: 25 个文件
  加载 right: 23 个文件
  ...

数据集大小: (120, 101, 13)

训练集: (96, 101, 13)
测试集: (24, 101, 13)

Epoch 1/30
3/3 ━━━━━━━━━━━━━━━━━━━━ 2s 10ms/step - accuracy: 0.85 - loss: 0.45
...
测试准确率: 93.75%
📁 输出文件
文件	说明
speech_cnn.h5	训练好的模型
training_history.png	训练曲线图
🕳️ 踩坑记录
坑1：MFCC特征形状对不上
现象：ValueError: Input 0 of layer "sequential" is incompatible with layer
原因：模型期望的输入形状和数据实际形状不匹配
解决：

python
# 提取MFCC后打印形状确认
mfcc = librosa.feature.mfcc(y=audio, sr=16000, n_mfcc=13, n_fft=512, hop_length=160)
print(mfcc.shape)  # 应该是 (13, 101)
mfcc = mfcc.T      # 转置成 (101, 13)
print(mfcc.shape)  # 应该是 (101, 13)
坑2：训练准确率低（<50%）
现象：loss降不下去，准确率一直在50%以下
原因：数据量太少（每个命令词只有几条）
解决：

python
# 方法1：数据增强
def augment_audio(audio, sr):
    # 加噪
    noise = np.random.normal(0, 0.005, audio.shape)
    audio_noise = audio + noise
    # 时间拉伸
    audio_stretch = librosa.effects.time_stretch(audio, rate=0.9)
    return [audio_noise, audio_stretch]

# 方法2：先拿公开数据集练手
# 用 train_with_public_data.py 验证代码没问题再用自己的数据
坑3：过拟合（训练集准确率99%，测试集60%）
现象：训练集准确率很高，测试集上表现差
原因：模型太复杂，数据量不够
解决：

python
# 增加Dropout
model.add(layers.Dropout(0.5))

# 增加L2正则化
layers.Conv1D(64, 3, activation='relu', kernel_regularizer='l2')

# 早停
early_stop = EarlyStopping(monitor='val_loss', patience=5, restore_best_weights=True)
坑4：训练时内存爆炸
现象：运行到一半电脑卡死，内存占用100%
原因：一次性加载所有音频到内存
解决：

python
# 用生成器逐批次加载
def data_generator(file_list, batch_size=32):
    while True:
        for i in range(0, len(file_list), batch_size):
            batch_files = file_list[i:i+batch_size]
            X_batch = []
            y_batch = []
            for f in batch_files:
                audio, _ = librosa.load(f, sr=16000)
                mfcc = librosa.feature.mfcc(y=audio, sr=16000, n_mfcc=13)
                X_batch.append(mfcc.T)
                y_batch.append(label)
            yield np.array(X_batch), np.array(y_batch)
