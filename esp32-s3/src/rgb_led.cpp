#include "rgb_led.h"
#include <FastLED.h>

// 定义灯珠数组
CRGB leds[RGB_COUNT];

void rgb_init()
{
  FastLED.addLeds<WS2812, RGB_PIN, RGB_ORDER>(leds, RGB_COUNT);
  rgb_off(); // 初始化关闭
  Serial.println("[RGB] WS2812 初始化完成");
}

void rgb_set_color(uint8_t r, uint8_t g, uint8_t b)
{
  for (int i = 0; i < RGB_COUNT; i++)
  {
    leds[i] = CRGB(r, g, b);
  }
}

void rgb_set_pixel(int index, uint8_t r, uint8_t g, uint8_t b)
{
  if (index >= 0 && index < RGB_COUNT)
  {
    leds[index] = CRGB(r, g, b);
  }
}

void rgb_show()
{
  FastLED.show();
}

void rgb_red()
{
  rgb_set_color(255, 0, 0);
  rgb_show();
}
void rgb_green()
{
  rgb_set_color(0, 255, 0);
  rgb_show();
}
void rgb_blue()
{
  rgb_set_color(0, 0, 255);
  rgb_show();
}
void rgb_white()
{
  rgb_set_color(255, 255, 255);
  rgb_show();
}
void rgb_off()
{
  rgb_set_color(0, 0, 0);
  rgb_show();
}