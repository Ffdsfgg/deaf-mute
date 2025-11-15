#pragma once
#include <Arduino.h>

// 按键编号定义
#define BTN_A 0
#define BTN_B 1
#define BTN_C 2
#define BTN_D 3

// 按键数量
#define BUTTON_COUNT 4

// 按键引脚定义
extern const int button_pins[BUTTON_COUNT];

// 初始化按键
void button_init();

// 更新所有按键状态（在loop中调用）
void button_update();

// 检查某个按键是否被按下
bool button_pressed(uint8_t btn);

// 获取按键当前电平（实时状态）
bool button_read(uint8_t btn);