# Copilot Instructions for ESP32-S3 PlatformIO Project

## 项目架构概览
- 本项目基于 PlatformIO，目标硬件为 Freenove ESP32-S3 WROOM，采用 Arduino 框架。
- 主要功能模块分布于 `src/` 目录，每个硬件功能（如 LED、光照、舵机、步进电机、OLED、按键、WiFi）均有独立的 `xxx.cpp`/`xxx.h` 文件。
- `main.cpp` 负责系统初始化与主循环，调用各模块接口实现传感器采集、外设控制与数据展示。

## 关键模块说明
- `led.*`：交通灯控制，提供红/黄/绿灯的开关函数。
- `light.*`：BH1750 光照传感器，I2C 通信，`light_read()` 返回 lux。
- `servo.*`：舵机控制，`servo_write(angle)` 控制角度。
- `stepper.*`：步进电机，`stepper_rotate_degrees(degrees, rpm)` 控制旋转。
- `oled.*`：OLED 显示，支持中文，`oled_printf_utf8` 用于格式化输出。
- `button.*`：按键管理，`button_update()` 必须在 loop 中调用，`button_pressed(btn)` 检测边沿触发。
- `wifiesp.*`：WiFi 连接，`wifi_init()`/`wifi_begin()` 初始化，`wifi_connected()` 检查连接。

## 构建与上传
- 使用 PlatformIO 工具链（VSCode 推荐），常用命令：
  - 构建：`pio run`
  - 上传：`pio run --target upload`
  - 串口监视：`pio device monitor`
- 配置文件为 `platformio.ini`，如需添加库请在 `lib_deps` 下声明。

## 约定与模式
- 各硬件模块均以 `xxx_init()` 初始化，所有状态更新/采集函数应在主循环中调用。
- 按键采用边沿检测，需每次循环调用 `button_update()`。
- OLED 显示采用缓冲区，需 `oled_update()` 刷新。
- 传感器与外设引脚定义集中在各自头文件，便于硬件适配。
- WiFi 配置硬编码于 `wifiesp.h`，如需安全请勿提交真实密码。

## 参考文件
- 主要入口：`src/main.cpp`
- 各模块接口：`src/*.h`、`src/*.cpp`
- 构建配置：`platformio.ini`

## 其他说明
- 本项目未包含测试用例，`test/` 目录为空。
- 如需扩展新硬件，建议仿照现有模块结构实现。

---
如有不明确之处，请查阅各模块头文件注释或补充本说明。