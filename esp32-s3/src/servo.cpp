#include "servo.h"
#include <ESP32Servo.h>

Servo my_servo; // 定义舵机对象

void servo_init()
{
  // 分配硬件定时器
  ESP32PWM::allocateTimer(0);
  // 设置PWM频率
  my_servo.setPeriodHertz(50);
  // 绑定引脚和脉宽范围
  my_servo.attach(SERVO_PIN, SERVO_MIN, SERVO_MAX);
  // 初始位置
  my_servo.write(0);
}

void servo_write(uint8_t angle)
{
  // 限制角度范围
  if (angle > 180)
    angle = 180;
  my_servo.write(angle);
}