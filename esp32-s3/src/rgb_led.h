#pragma once
#include <Arduino.h>

// 单线 RGB LED 配置
#define RGB_PIN 2     // 数据引脚
#define RGB_COUNT 4   // 灯珠数量
#define RGB_ORDER GRB // 颜色顺序

// 初始化 LED
void rgb_init();

// 设置指定位置灯珠颜色（0~255）
void rgb_set_color(uint8_t r, uint8_t g, uint8_t b);

// 设置第 n 个灯珠颜色（支持多个级联）
void rgb_set_pixel(int index, uint8_t r, uint8_t g, uint8_t b);

// 显示颜色
void rgb_show();

// 快捷颜色
void rgb_red();
void rgb_green();
void rgb_blue();
void rgb_white();
void rgb_off();