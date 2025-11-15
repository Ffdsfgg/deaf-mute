#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include <HTTPClient.h>

#include "light.h"
#include "servo.h"
#include "wifiesp.h"
#include "rgb_led.h"
#include "UTF8ToGB2312.h"

// DHT
#define DHT_PIN 13
#define DHT_TYPE DHT11
DHT dht(DHT_PIN, DHT_TYPE);

//MQTT
WiFiClient espClient;
PubSubClient mqtt(espClient);
const char *MQTT_SERVER = "119.29.176.83";
const int MQTT_PORT = 1883;
const char *CLIENT_ID = "esp32_all_in_one";
#define TOPIC_STATUS "esp32/status"
#define TOPIC_CMD "esp32/command"

//定时上报
unsigned long lastReport = 0;
const unsigned long REPORT_INTERVAL = 2000;

//蜂鸣器
#define BUZZER_PIN 21
void buzzer_init()
{
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
}
void buzzer_on()
{
  digitalWrite(BUZZER_PIN, HIGH);
  Serial.println("[Buzzer] ON");
}
void buzzer_off()
{
  digitalWrite(BUZZER_PIN, LOW);
  Serial.println("[Buzzer] OFF");
}

void speech(const String &text)
{
  if (text.length() == 0)
    return;

  //UTF-8 转 GB2312 
  String gb2312 = GB.get(text);
  int dataLen = gb2312.length();
  if (dataLen == 0)
  {
    Serial.println("GB2312 转换失败，跳过本次播报");
    return;
  }

  // 组帧 
  int frameLen = dataLen + 6; // 帧头3B
  uint8_t head[frameLen];
  head[0] = 0xFD;        // 帧头
  head[1] = 0x00;        // 长度高字节
  head[2] = dataLen + 3; // 长度低字节
  head[3] = 0x01;        // 合成命令
  head[4] = 0x00;        // 默认参数

  memcpy(&head[5], gb2312.c_str(), dataLen);

  // 校验 
  head[frameLen - 1] = 0;
  for (int i = 0; i < frameLen - 1; ++i)
    head[frameLen - 1] ^= head[i];
  //发送
  Serial2.write(head, frameLen);
  Serial.printf("[Speech] > %s  (GB2312:%dB)\n", text.c_str(), dataLen);
}

// JSON 命令解析
void execCmd(const String &json)
{
  JsonDocument doc;
  auto err = deserializeJson(doc, json);
  if (err)
  {
    Serial.printf("JSON 解析错误: %s\n", err.c_str());
    return;
  }

  const char *cmd = doc["cmd"];
  if (!cmd)
    return;

  // RGB LED
  if (strcmp(cmd, "rgb") == 0)
  {
    const char *v = doc["val"];
    if (strcmp(v, "red") == 0)
    {
      rgb_red();
      speech("警告警告，检测到红色警报信号");
    }
    else if (strcmp(v, "green") == 0)
    {
      rgb_green();
      speech("系统状态正常，显示绿色指示灯");
    }
    else if (strcmp(v, "blue") == 0)
    {
      rgb_blue();
      speech("设备掉线，显示蓝色指示灯");
    }
    else if (strcmp(v, "white") == 0)
      rgb_white();
    else if (strcmp(v, "off") == 0)
      rgb_off();
    return;
  }

  // 交通灯
  if (strcmp(cmd, "traffic") == 0)
  {
    const char *v = doc["val"];
    if (strcmp(v, "red") == 0)
      rgb_red();
    else if (strcmp(v, "yellow") == 0)
    {
      Serial.println("黄灯（未实现）");
    }
    else if (strcmp(v, "green") == 0)
      rgb_green();
    else
      Serial.printf("未知交通灯状态: %s\n", v);
    return;
  }

  // 舵机
  if (strcmp(cmd, "servo") == 0)
  {
    {
    }

    int angle = doc["angle"] | 90;
    if (angle == 0)
    {
      speech("已关闭水闸");
    }
    else
    {
      speech("已开启水闸");
    }

    servo_write(constrain(angle, 0, 180));
    return;
  }

  // 蜂鸣器
  if (strcmp(cmd, "buzzer") == 0)
  {
    const char *v = doc["val"];
    if (strcmp(v, "on") == 0)
      buzzer_on();
    else if (strcmp(v, "off") == 0)
      buzzer_off();
    else
      Serial.printf("未知蜂鸣器指令: %s\n", v);
    return;
  }

  // 语音播报
  if (strcmp(cmd, "speak") == 0)
  {
    const char *txt = doc["text"];
    speech(txt ? txt : "未提供播报内容");
    return;
  }

}

//MQTT 回调
void mqttCallback(char *topic, byte *payload, unsigned int len)
{
  String json = "";
  for (uint16_t i = 0; i < len; ++i)
    json += (char)payload[i];
  Serial.printf("[MQTT RX] %s -> %s\n", topic, json.c_str());
  execCmd(json);
}

//上报传感器
void reportStatus()
{
  float t = dht.readTemperature();
  float h = dht.readHumidity();
  float lux = light_read();


  JsonDocument doc;
  doc["temp"] = t;
  doc["hum"] = h;
  doc["lux"] = lux;
  char buf[128];
  serializeJson(doc, buf);
  mqtt.publish(TOPIC_STATUS, buf);
  Serial.printf("[Report] 温度:%.1f℃ 湿度:%.1f%% 光照:%.1f lux\n", t, h, lux);

}

//初始化
void setup()
{
  Serial.begin(115200); 
  Serial2.begin(9600, SERIAL_8N1, 5, 4); 
  delay(1000);

  // 外设初始化
  wifi_init();
  wifi_begin();

  rgb_init();
  light_init();
  dht.begin();
  servo_init();
  buzzer_init();

  // MQTT 设置
  mqtt.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt.setCallback(mqttCallback);

  delay(500);
  speech("系统启动成功，语音模块已就绪。");
}

// 主循环
void loop()
{
  // MQTT 连接管理
  if (!mqtt.connected() && wifi_connected())
  {
    if (mqtt.connect(CLIENT_ID,"root", "root"))
    {
      mqtt.subscribe(TOPIC_CMD);
      Serial.println("[MQTT] 已连接并订阅命令");
    }
    else
    {
      delay(5000);
    }
  }

  if (mqtt.connected())
  {
    mqtt.loop();
  }

  // 定时上报
  if (millis() - lastReport >= REPORT_INTERVAL)
  {
    lastReport = millis();
    reportStatus();
  }

  delay(10);
}