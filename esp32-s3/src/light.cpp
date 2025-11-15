#include "light.h"
#include <Wire.h>
#include <BH1750.h>

// 创建 BH1750 对象
BH1750 lightMeter;

//初始化光照传感器
bool light_init() {
  // 初始化第二组 I2C
  Wire1.begin(LIGHT_SDA_PIN, LIGHT_SCL_PIN);

  // 初始化 BH1750
  if (lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, LIGHT_I2C_ADDR, LIGHT_WIRE_ID)) {
    Serial.println("光照传感器初始化成功");
    return true;
  } else {
    Serial.println("光照传感器初始化失败");
    return false;
  }
}

//读取光照强度 lux 值，失败返回 -1
float light_read() {
  float lux = lightMeter.readLightLevel();
  
  // 异常值
  if (isnan(lux)) {
    return -1;
  }

  return lux;
}