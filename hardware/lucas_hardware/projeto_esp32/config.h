#ifndef CONFIG_H
#define CONFIG_H

/*
 * config.h — TODA a configuracao do projeto em um so lugar.
 * Mude aqui sem precisar mexer na logica do firmware.
 */
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ---------- Wi-Fi e endpoint ----------
// Credenciais e IP do backend ficam no secrets.h (fora do git).
// Copie secrets.example.h para secrets.h e ajuste para a sua rede.
#include "secrets.h"

// ---------- OLED ----------
#define LARGURA 128
#define ALTURA 64
#define OLED_ADDR 0x3C
// Pinos I2C remapeados (lado esquerdo do board):
#define PINO_SDA 25
#define PINO_SCL 26

// ---------- LED RGB (catodo comum: comum no GND, acende em HIGH) ----------
// So 2 cores ligadas, em D32 e D33.
#define PINO_LED_A        32
#define PINO_LED_B        33

// Canais PWM (LEDC) — cada saida PWM precisa de um canal proprio
#define CH_LED_A   0
#define CH_LED_B   1
#define CH_BUZZER  2

// ---------- Potenciometros ----------
#define PINO_POT_ENERGIA  36   // ADC1 - VP
#define PINO_POT_SOM      39   // ADC1 - VN

// ---------- Buzzer ----------
#define PINO_BUZZER       13

// ---------- Limiar para considerar "ligado" ----------
// Como nao ha interruptor fisico, usamos o pot de energia:
// acima deste valor (0-4095) o sistema conta como ligado.
#define LIMIAR_LIGADO     200

// ---------- Tempos (em milissegundos) ----------
#define INTERVALO_ENVIO   2000   // de quanto em quanto tempo manda dados pro app
#define INTERVALO_LEITURA 100    // de quanto em quanto tempo le os sensores

// ---------- Filtro ----------
#define TAMANHO_JANELA    10     // N do filtro de media movel

#endif