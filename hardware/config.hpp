#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <BluetoothSerial.h>

#define WIFI_SSID "SUA_REDE"
#define WIFI_PASSWORD "SUA_SENHA"

#define SERVER_HOST "192.168.0.10"
#define SERVER_PORT 3000
#define SERVER_PATH "/data"
#define SERVER_CONFIG_PATH "/config"

#define BT_NAME "ESP32_Estabilizador"

#define CONNECTION_ID 1

#define PIN_POT_RECEIVED 36
#define PIN_POT_BASE 39
#define PIN_BUTTON 4
#define PIN_BUZZER 13
#define PIN_OUTPUT 25

#define PWM_CH_OUTPUT 0
#define PWM_CH_BUZZER 1
#define PWM_FREQ 5000
#define PWM_RES 8

#define ADC_MAX 4095
#define PWM_MAX 255

#define SEND_INTERVAL 2000
#define READ_INTERVAL 100
#define RECONNECT_INTERVAL 5000
#define DEBOUNCE_INTERVAL 200

#define WINDOW_SIZE 10
#define STABLE_THRESHOLD 150
#define ALERT_TONE 1200
#define BEEP_DURATION 60

#endif
