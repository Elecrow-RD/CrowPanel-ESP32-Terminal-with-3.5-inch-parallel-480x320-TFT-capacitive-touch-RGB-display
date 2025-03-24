from machine import Pin, PWM
import time

# 定义蜂鸣器引脚
BUZZER_PIN = 20

# 初始化PWM
buzzer = PWM(Pin(BUZZER_PIN))
buzzer.freq(1000)  # 设置初始频率为1kHz

def play_tone(frequency, duration):
    buzzer.freq(frequency)
    buzzer.duty(512)  # 50% 占空比
    time.sleep(duration)
    buzzer.duty(0)  # 关闭蜂鸣器

def play_melody(melody):
    for tone, duration in melody:
        if tone == 0:
            time.sleep(duration)  # 休止符
        else:
            play_tone(tone, duration)

# 定义一个简单的旋律（音调和时长）
melody = [
    (262, 0.5),  # C4
    (294, 0.5),  # D4
    (330, 0.5),  # E4
    (349, 0.5),  # F4
    (392, 0.5),  # G4
    (440, 0.5),  # A4
    (494, 0.5),  # B4
    (523, 0.5),  # C5
    (0, 0.5),    # 休止符
]

# 播放旋律
play_melody(melody)

# 清理
buzzer.deinit()
