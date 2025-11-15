#include "stepper.h"
#include <CheapStepper.h>

// 创建步进电机对象
CheapStepper stepper(STEPPER_IN1, STEPPER_IN2, STEPPER_IN3, STEPPER_IN4);

//初始化步进电机
void stepper_init() {
  // 设置默认转速
  stepper.setRpm(10);
}

//控制步进电机旋转指定角度 degrees 目标角度（正数=顺时针，负数=逆时针） rpm 转速（可选，默认10）
void stepper_rotate_degrees(int degrees, uint8_t rpm) {
  // 如果角度为0，直接返回
  if (degrees == 0) {
    return;
  }
  // 设置转速
  stepper.setRpm(rpm);

  // 判断方向：正数为顺时针，负数为逆时针
  bool clockwise = degrees > 0;

  //使用库支持的 moveDegrees 函数（自动换算步数）
  stepper.moveDegrees(clockwise, abs(degrees));
}