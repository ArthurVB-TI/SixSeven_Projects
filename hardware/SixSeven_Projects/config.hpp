#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>

// =====================================================================
//  CONFIG PARA ESP32-S3
//  Regras de pinagem do S3:
//   - Potenciometros (analogico) SO em ADC1 = GPIO1..GPIO10 (funcionam
//     com Wi-Fi ligado). No S3, ADC2 nao funciona com Wi-Fi.
//   - Evitar: GPIO0/3/45/46 (strapping), GPIO19/20 (USB), GPIO26-32
//     e vizinhos (flash/PSRAM), GPIO6-11 (flash).
// =====================================================================

// ---------- Wi-Fi ----------
#define WIFI_SSID "IZAMARAWIFI_2.4"
#define WIFI_PASSWORD "@Ia020180"

// ---------- Backend (o que conversa com o Drogon) ----------
#define SERVER_HOST "192.168.18.5"
#define SERVER_PORT 3000
#define SERVER_PATH "/data"          // POST das leituras
#define SERVER_CONFIG_PATH "/config" // GET da configuracao (E_b, timer)

// ---------- Identidade desta conexao/hardware ----------
#define CONNECTION_ID 1

// ---------- Pinos (ESP32-S3) ----------
// Potenciometros em ADC1 (GPIO1/GPIO2): unicos que leem certo com Wi-Fi no S3.
#define PIN_POT_RECEIVED 1   // energia recebida (E_r)
#define PIN_POT_BASE 2       // energia base (E_b)
#define PIN_BUTTON 15        // botao unico (INPUT_PULLUP)
#define PIN_BUZZER 9
#define PIN_LED_A 11
#define PIN_LED_B 12

#define PIN_OLED_SCL 4
#define PIN_OLED_SDA 5

// ---------- PWM (LEDC) ----------
#define PWM_FREQ 5000
#define PWM_RES 8

#define ADC_MAX 4095
#define PWM_MAX 255

// ---------- Tempos (ms) ----------
#define SEND_INTERVAL 10000
#define READ_INTERVAL 1000
#define RECONNECT_INTERVAL 5000
#define DEBOUNCE_INTERVAL 200
#define BOOT_WIFI_WAIT 3000
#define HTTP_TIMEOUT 1500

// ---------- Filtro / logica ----------
#define WINDOW_SIZE 10
#define STABLE_THRESHOLD 150
#define LED_LOW 1500
#define LED_HIGH 2800
#define ALERT_TONE 1200
#define BEEP_DURATION 60

#endif
