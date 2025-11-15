#ifndef __LED_H
#define __LED_H

#define LED_RED_PIN     3
#define LED_YELLOW_PIN  45
#define LED_GREEN_PIN   46

//初始化
void led_init();
//红灯点亮
void led_red_on();
//红灯熄灭
void led_red_off();
//黄灯点亮
void led_yellow_on();
//黄灯熄灭
void led_yellow_off();
//绿灯点亮
void led_green_on();
//绿灯熄灭
void led_green_off();

#endif