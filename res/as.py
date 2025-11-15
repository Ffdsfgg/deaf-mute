import time
import sys
import sounddevice as sd
import numpy as np
import nls
import json
import os
URL = "wss://nls-gateway-cn-shanghai.aliyuncs.com/ws/v1"
TOKEN = "XXX"
APPKEY = "XXX"
class MicrophoneRecognizer:
    def __init__(self):
        self.recording = False
        self.sr = None

    def start(self):
        print("说点什么吧...")
        self.recording = True

        # 初始化识别客户端
        self.sr = nls.NlsSpeechTranscriber(
            url=URL,
            token=TOKEN,
            appkey=APPKEY,
            on_sentence_end=self.on_sentence_end,  # 关心最终结果
            on_error=self.on_error,
            on_close=self.on_close
        )

        # 启动会话
        self.sr.start(
            aformat="pcm",
            enable_intermediate_result=False,       # 关闭中间结果（更干净）
            enable_punctuation_prediction=True,     # 加标点
            enable_inverse_text_normalization=True  # 数字转文字
        )

        def audio_callback(indata, frames, time, status):
            if status:
                print("音频错误:", status)
            if self.recording:
                # 转为 PCM 字节流
                pcm_data = (indata[:, 0] * 32767).astype(np.int16).tobytes()
                self.sr.send_audio(pcm_data)

        # 开始录音
        with sd.InputStream(samplerate=16000, channels=1, dtype='float32', blocksize=640, callback=audio_callback):
            while self.recording:
                time.sleep(0.1)

    def stop(self):
        self.recording = False
        if self.sr:
            self.sr.stop()
        print("\n再见！")

    def on_sentence_end(self, message, *args):
        try:
            if isinstance(message, str):
                message = json.loads(message)
            result = message.get("payload", {}).get("result", "").strip()
            if not result:
                return
            print("你说：", result)
            # 显示你说的话
            cmd_echo = f'espeak -v zh -s 150  "{result}"  --stdout | aplay'
            os.system(cmd_echo)

            # === 可选：添加语音控制命令 ===
            if "关机" in result:
                os.system('espeak -v zh -s 150   "即将关机..." && sudo shutdown now')
            elif "重启" in result:
                os.system('espeak -v zh -s 150   "即将重启..." && sudo reboot')
            elif "播放音乐" in result:
                os.system('espeak -v zh -s 150   "播放音乐..." && mpg123 /home/pi/music.mp3 &')
            elif "你好" in result:
                os.system('espeak -v zh -s 150   "你好呀！"  --stdout | aplay')  # 需安装 festival
        except Exception as e:
            print(" 解析结果失败:", str(e))
    def on_error(self, message, *args):
        print("识别出错:", message.get("header", {}).get("status_text", "未知错误"))

    def on_close(self, *args):
        print(" 连接已关闭")

def main():
    recognizer = MicrophoneRecognizer()
    try:
        recognizer.start()
    except KeyboardInterrupt:
        recognizer.stop()
    except Exception as e:
        print("异常:", str(e))
        recognizer.stop()

if __name__ == "__main__":
    main()