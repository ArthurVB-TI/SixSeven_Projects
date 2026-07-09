#ifndef CONFIG_H
#define CONFIG_H

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// =====================================================================
//  CONFIG PARA ESP32-S3  (pinagem revisada p/ as regras do S3)
//  Regras que guiaram a escolha dos pinos:
//   - Potenciometros (analogico) SO em ADC1 = GPIO1..GPIO10 (funcionam
//     com Wi-Fi ligado). No S3, ADC2 nao funciona com Wi-Fi.
//   - Evitar: GPIO0/3/45/46 (strapping), GPIO19/20 (USB), GPIO26-32
//     e vizinhos (flash/PSRAM), GPIO6-11 (flash).
// =====================================================================

// ---------- Wi-Fi ----------
// Descomente e ajuste para religar o Wi-Fi:
#define WIFI_SSID     "IZAMARAWIFI_2.4"
#define WIFI_SENHA    "@Ia020180"

// ---------- Endpoint que recebe os dados ----------
#define URL_DESTINO   "http://192.168.18.5:3000/dados"

// ---------- LED RGB (3 cores: A, B, C) ----------
// Catodo comum: comum no GND, cada cor acende em HIGH.
#define PINO_LED_A        11   // cor A
#define PINO_LED_B        12   // cor B
#define PINO_LED_C        17   // cor C

// ---------- BUZZER passivo ----------
#define PINO_BUZZER       9    // seguro no S3 (PWM/LEDC por pino)

// ---------- POTENCIOMETROS (entradas analogicas) ----------
// MOVIDOS para ADC1 (GPIO1/GPIO2): unicos que leem certo com Wi-Fi no S3.
#define PINO_POT_ENERGIA  1    // ADC1_CH0 - meio do potenciometro de energia
#define PINO_POT_SOM      2    // ADC1_CH1 - meio do potenciometro de som

// ---------- BOTAO unico ----------
#define PINO_BOTAO        15   // entrada digital com INPUT_PULLUP

// ---------- Limiar para considerar "ligado" ----------
// Sem interruptor fisico: acima deste valor (0-4095) conta como ligado.
#define LIMIAR_LIGADO     200

// ---------- Tela OLED (I2C - SSD1306) ----------
#define PINO_OLED_SDA     5
#define PINO_OLED_SCL     4
#define OLED_LARGURA      128    // pixels na horizontal
#define OLED_ALTURA       64     // pixels na vertical (use 32 se a tela for menor)
#define OLED_ENDERECO     0x3C   // endereco I2C tipico (alguns modulos usam 0x3D)

// ---------- Tempos (em milissegundos) ----------
#define INTERVALO_ENVIO   2000   // de quanto em quanto tempo manda dados
#define INTERVALO_LEITURA 100    // de quanto em quanto tempo le os sensores
#define INTERVALO_OLED    400    // de quanto em quanto tempo redesenha o OLED (I2C lento)

// ---------- Filtro ----------
#define TAMANHO_JANELA    10     // N do filtro de media movel

#endif