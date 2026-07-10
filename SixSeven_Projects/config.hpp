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
// ATENCAO: o ESP32-S3 so conecta em redes 2.4 GHz. Um SSID "_5G"
// nunca vai conectar — use a rede 2.4 do mesmo roteador.
#define WIFI_SSID "MUNOZ-COELHO"
#define WIFI_PASSWORD "Guacamole23!"

// ---------- Backend (o que conversa com o Drogon) ----------
// Este cliente fala HTTP cru (sem TLS e sem seguir redirect), entao:
//  - NAO usar a porta 3000 do host: e o painel do EasyPanel.
//  - NAO usar 80/443: o dominio redireciona para HTTPS.
//  - Caminho certo: porta publicada no servico do backend no EasyPanel
//    (Ports: host 3100 -> container 3000) + liberar 3100/tcp no firewall.
// O contrato e POST /data + GET /config — NAO trocar por /dados
// (isso e do outro backend, incompativel com este firmware).
#define SERVER_HOST "sixsevenapi.nerv3.xyz"
#define SERVER_PORT 3100
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
// LED RGB catodo comum (comum no GND, cor acende em HIGH).
#define PIN_LED_A 11
#define PIN_LED_B 12
#define PIN_LED_C 17         // 3a cor do RGB (hardware novo; ainda sem uso no firmware)

#define PIN_OLED_SCL 4
#define PIN_OLED_SDA 5
#define OLED_WIDTH 128       // resolucao do SSD1306
#define OLED_HEIGHT 64
#define OLED_ADDRESS 0x3C    // endereco I2C (alguns modulos usam 0x3D)
#define OLED_INTERVAL 400    // ms entre redesenhos (I2C e lento)

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
