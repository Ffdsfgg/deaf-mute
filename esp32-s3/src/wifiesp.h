#pragma once
#include <Arduino.h>

// WiFi 配置
#define WIFI_SSID "20032516"
#define WIFI_PASS "P@ssW0rd"

// 初始化 WiFi
void wifi_init();

// 初始化 WiFi
void wifi_begin();

// 在 loop() 中调用，更新连接状态
void wifi_update();

// 检查是否已连接
bool wifi_connected();

// 获取本地 IP
String wifi_ip();