import cv2
import time
import numpy as np
import json
import base64
from collections import deque
from aip import AipBodyAnalysis
import mediapipe as mp
from PIL import Image, ImageDraw, ImageFont

#MQTT 配置
import paho.mqtt.client as mqtt_client

MQTT_BROKER = "82.157.30.217"
MQTT_PORT = 1883
MQTT_TOPIC = "device/vocality"
MQTT_USER = "XXX"
MQTT_PASS = "XXX"

# 全局 MQTT 客户端
mqtt_client_instance = None


def connect_mqtt():
    """连接到 MQTT 服务器"""
    global mqtt_client_instance
    client = mqtt_client.Client()
    client.username_pw_set(MQTT_USER, MQTT_PASS)

    def on_connect(cl, userdata, flags, rc):
        if rc == 0:
            print("MQTT 已连接到服务器")
        else:
            print(f"MQTT 连接失败，代码 {rc}")

    client.on_connect = on_connect
    try:
        client.connect(MQTT_BROKER, MQTT_PORT, 60)
        return client
    except Exception as e:
        return None


def publish_mqtt(text):
    """发布消息到 MQTT"""
    global mqtt_client_instance
    if mqtt_client_instance is None:
        print("MQTT 客户端未连接")
        return

    payload = json.dumps({"text": text}, ensure_ascii=False)
    result = mqtt_client_instance.publish(MQTT_TOPIC, payload)
    status = result[0]
    if status == 0:
        print(f"已发布到 MQTT: {payload}")

#百度api

APP_ID = 'XXX'
API_KEY = 'XXX'
SECRET_KEY = 'XXX'

gesture_client = AipBodyAnalysis(APP_ID, API_KEY, SECRET_KEY)

#MediaPipe 手部方向检测
mp_hands = mp.solutions.hands.Hands(
    static_image_mode=False,
    max_num_hands=1,
    min_detection_confidence=0.5
)

#手语词汇表
SIGN_LANGUAGE_VOCAB = {
    'ILY': '我爱你',
    'Prayer': '谢谢',
    'Thumb_up': '好',
    'Eight': '你们',
    'Fist': '今天',
    'Heart_single': '喜欢',
    'Three': '三',
    'Honour': '再见',
    'Seven': '星期'
}

#中文字体支持
FONT_PATH = "/root/pytest/gesture/font/NotoSansCJK-Regular.otf"  # 修改为你的实际路径


def cv2AddChineseText(img, text, pos, color=(0, 255, 0), size=28):
    img_pil = Image.fromarray(cv2.cvtColor(img, cv2.COLOR_BGR2RGB))
    draw = ImageDraw.Draw(img_pil)
    try:
        font = ImageFont.truetype(FONT_PATH, size, encoding="utf-8")
    except Exception:
        font = ImageFont.load_default()
    draw.text(pos, text, fill=color, font=font)
    return cv2.cvtColor(np.asarray(img_pil), cv2.COLOR_RGB2BGR)


#手掌方向检测
def palm_direction(frame):
    rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    res = mp_hands.process(rgb)
    if res.multi_hand_landmarks:
        lm = res.multi_hand_landmarks[0].landmark
        dx = lm[12].x - lm[0].x
        if dx > 0.05:
            return "right"
        elif dx < -0.05:
            return "left"
    return None


#手语识别类
class SignLanguageRecognizer:
    def __init__(self, vocab=None, debounce_ms=800, sentence_timeout=2.0):
        self.vocab = vocab or SIGN_LANGUAGE_VOCAB
        self.debounce_ms = debounce_ms
        self.sentence_timeout = sentence_timeout
        self.sentence = deque(maxlen=10)
        self.last_gesture = None
        self.last_gesture_time = 0
        self.last_active_time = time.time()

    def recognize_frame(self, frame):
        image_data = cv2.imencode('.jpg', frame)[1].tobytes()
        try:
            result = gesture_client.gesture(image_data)
            if 'result' not in result or not result['result']:
                return False

            gesture_name = result['result'][0]['classname']
            if gesture_name not in self.vocab:
                return False

            word = self.vocab[gesture_name]
            if word == '你们':
                dire = palm_direction(frame)
                word = '给我们' if dire == 'left' else '给你们' if dire == 'right' else '你们'

            current_ms = int(time.time() * 1000)
            if (word != self.last_gesture and
                    (current_ms - self.last_gesture_time) > self.debounce_ms):
                self.sentence.append(word)
                print(f"手势识别: {word} → 当前句子: {''.join(self.sentence)}")

                self.last_gesture = word
                self.last_gesture_time = current_ms
                self.last_active_time = time.time()
                return True

        except Exception as e:
            print("手势识别错误:", e)
        return False

    def get_sentence(self):
        return ''.join(list(self.sentence))

    def is_complete(self):
        if len(self.sentence) == 0:
            return False
        return (time.time() - self.last_active_time) > self.sentence_timeout

    def clear(self):
        self.sentence.clear()
        self.last_gesture = None


#主函数
def main():
    global mqtt_client_instance
    print("启动手语识别系统...")

    # 初始化 MQTT
    mqtt_client_instance = connect_mqtt()
    # 启动 MQTT 循环
    mqtt_client_instance.loop_start()

    # 打开摄像头
    cap = cv2.VideoCapture(0)
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, 640)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 480)

    recognizer = SignLanguageRecognizer()

    try:
        while True:
            ret, frame = cap.read()
            if not ret:
                continue

            # 手势识别
            recognizer.recognize_frame(frame)

            # 显示当前句子
            h, w = frame.shape[:2]
            cv2.rectangle(frame, (0, h - 60), (w, h), (0, 0, 0), -1)
            sentence_text = recognizer.get_sentence()
            frame = cv2AddChineseText(frame, sentence_text, (10, h - 45), size=28)

            cv2.imshow('Sign Language Recognizer [Press Q to Exit]', frame)

            # 句子完成，发送 MQTT
            if recognizer.is_complete():
                final = recognizer.get_sentence()
                if final.strip():
                    print(f"完整句子识别: {final}")
                    publish_mqtt(final)  # 发送到 MQTT
                recognizer.clear()

            if cv2.waitKey(1) == ord('q'):
                break

    except KeyboardInterrupt:
        print("\n程序被用户中断")

    finally:
        # 清理资源
        cap.release()
        cv2.destroyAllWindows()
        mqtt_client_instance.loop_stop()
        print("系统已关闭")


if __name__ == "__main__":
    main()