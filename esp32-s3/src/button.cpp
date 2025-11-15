#include "button.h"
#include <Arduino.h>

// 按键引脚分配
const int button_pins[BUTTON_COUNT] = {8, 9, 15, 16};

// 按键状态
static bool btn_state[BUTTON_COUNT];        // 当前读取的电平
static bool btn_last_state[BUTTON_COUNT];   // 上一次电平
static bool btn_pressed_flag[BUTTON_COUNT]; // 是否已触发按下事件
static unsigned long btn_debounce_time[BUTTON_COUNT]; // 防抖时间

// 初始化所有按键为上拉输入
void button_init() {
  for (int i = 0; i < BUTTON_COUNT; i++) {
    pinMode(button_pins[i], INPUT_PULLUP);// 内部上拉，按键接地
    btn_state[i] = true;
    btn_last_state[i] = true;
    btn_pressed_flag[i] = false;
    btn_debounce_time[i] = 0;
  }
}

// 更新所有按键状态
void button_update() {
  unsigned long now = millis();
  for (int i = 0; i < BUTTON_COUNT; i++) {
    bool reading = digitalRead(button_pins[i]);

    // 防抖：只有状态变化持续 50ms 才认为有效
    if (reading != btn_state[i]) {
      if (now - btn_debounce_time[i] > 50) {
        btn_state[i] = reading;
        btn_debounce_time[i] = now;
      }
    }

    // 检测按下
    if (!btn_state[i] && btn_last_state[i]) {
      btn_pressed_flag[i] = true;
    }

    // 更新上一次状态
    btn_last_state[i] = btn_state[i];
  }
}

// 检查按键是否被按下（只触发一次）
bool button_pressed(uint8_t btn) {
  if (btn >= BUTTON_COUNT) return false;
  if (btn_pressed_flag[btn]) {
    btn_pressed_flag[btn] = false;  //防止重复触发
    return true;
  }
  return false;
}

// 获取按键当前状态
bool button_read(uint8_t btn) {
  if (btn >= BUTTON_COUNT) return false;
  return !btn_state[btn];
}