#pragma once
#include <Arduino.h>

//步进电机引脚定义
#define STEPPER_IN1  7
#define STEPPER_IN2  6
#define STEPPER_IN3  21
#define STEPPER_IN4  17

//每圈总步数
#define STEPPER_STEPS_PER_REVOLUTION 2048

//初始化步进电机
void stepper_init();

//控制步进电机旋转指定角度 degrees 目标角度（正数=顺时针，负数=逆时针） rpm 转速（可选，默认10）
void stepper_rotate_degrees(int degrees, uint8_t rpm = 10);