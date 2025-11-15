#ifndef LIGHT_H
#define LIGHT_H

#include <Arduino.h>

// I2C地址和引脚定义
#define LIGHT_I2C_ADDR    0x23
#define LIGHT_SDA_PIN     41
#define LIGHT_SCL_PIN     42
#define LIGHT_WIRE_ID     &Wire1

//初始化 BH1750 光照传感器
bool light_init();

//读取当前光照强度 光照值
float light_read();

#endif