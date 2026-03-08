import sounddevice as sd
print(sd.query_devices())
print(f"默认输入设备: {sd.default.device[0]}")