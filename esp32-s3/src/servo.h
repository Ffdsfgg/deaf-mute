#pragma once
#include <Arduino.h>

// 舵机引脚定义
#define SERVO_PIN   18
#define SERVO_MIN   500   // 最小脉宽
#define SERVO_MAX   2500  // 最大脉宽

//初始化舵机
void servo_init();

//控制舵机转动角度（0~180）
void servo_write(uint8_t angle);