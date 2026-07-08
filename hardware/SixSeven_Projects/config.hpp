#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

#define WIFI_SSID "SUA_REDE"
#define WIFI_PASSWORD "SUA_SENHA"

#define SERVER_HOST "192.168.0.10"
#define SERVER_PORT 3000
#define SERVER_PATH "/data"
#define SERVER_CONFIG_PATH "/config"

#define CONNECTION_ID 1

#define PIN_POT_RECEIVED 36
#define PIN_POT_BASE 39
#define PIN_BUTTON 27
#define PIN_BUZZER 13
#define PIN_LED_A 33
#define PIN_LED_B 32

#define PIN_OLED_SCL 18
#define PIN_OLED_SDA 19

#define PWM_FREQ 5000
#define PWM_RES 8

#define ADC_MAX 4095
#define PWM_MAX 255

#define SEND_INTERVAL 10000
#define READ_INTERVAL 1000
#define RECONNECT_INTERVAL 5000
#define DEBOUNCE_INTERVAL 200
#define BOOT_WIFI_WAIT 3000
#define HTTP_TIMEOUT 1500

#define WINDOW_SIZE 10
#define STABLE_THRESHOLD 150
#define LED_LOW 1500
#define LED_HIGH 2800
#define ALERT_TONE 1200
#define BEEP_DURATION 60

#endif
