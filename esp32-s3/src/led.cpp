#include "led.h"
#include <Arduino.h>

void led_init() {
    pinMode(LED_RED_PIN, OUTPUT);
    pinMode(LED_YELLOW_PIN, OUTPUT);
    pinMode(LED_GREEN_PIN, OUTPUT);
}

void led_red_on() {
    digitalWrite(LED_RED_PIN, HIGH);
}

void led_red_off() {
    digitalWrite(LED_RED_PIN, LOW);
}

void led_yellow_on() {
    digitalWrite(LED_YELLOW_PIN, HIGH);
}

void led_yellow_off() {
    digitalWrite(LED_YELLOW_PIN, LOW);
}

void led_green_on() {
    digitalWrite(LED_GREEN_PIN, HIGH);
}

void led_green_off() {
    digitalWrite(LED_GREEN_PIN, LOW);
}