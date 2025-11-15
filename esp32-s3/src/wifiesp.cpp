#include "wifiesp.h"
#include <WiFi.h>

// 当前 WiFi 状态
static bool wifi_is_connected = false;
static bool wifi_is_connecting = false;
static unsigned long connect_start_time;

void wifi_init() {
  // 设置为 STA 模式
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  Serial.printf("[WiFi] 正在连接热点: %s\n", WIFI_SSID);

  // 阻塞等待连接
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout++ < 60) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    wifi_is_connected = true;
    Serial.println("\n[WiFi] 连接成功！");
    Serial.printf("[WiFi] IP: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("\n[WiFi] 连接失败或超时！");
  }
}

void wifi_begin() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  wifi_is_connecting = true;
  connect_start_time = millis();
  Serial.printf("[WiFi] 开始异步连接: %s\n", WIFI_SSID);
}

void wifi_update() {
  if (wifi_is_connecting) {
    if (WiFi.status() == WL_CONNECTED) {
      wifi_is_connected = true;
      wifi_is_connecting = false;
      Serial.printf("[WiFi] 连接成功！IP: %s\n", WiFi.localIP().toString().c_str());
    }
    else if (millis() - connect_start_time > 10000) { // 10秒超时
      wifi_is_connecting = false;
      Serial.printf("[WiFi] 连接超时: %s\n", WIFI_SSID);
    }
  }
}

bool wifi_connected() {
  return wifi_is_connected;
}

String wifi_ip() {
  if (wifi_connected()) {
    return WiFi.localIP().toString();
  }
  return "0.0.0.0";
}