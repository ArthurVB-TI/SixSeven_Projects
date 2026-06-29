/*
 * ============================================================================
 *  PROJETO INTEGRADOR I - Parte Hardware (ESP32)
 *  Telemetria de sensores via Wi-Fi com transporte desacoplado.
 * ============================================================================
 *
 *  HARDWARE REAL DESTA MONTAGEM:
 *    - 2 potenciometros: VP (GPIO36) e VN (GPIO39)
 *    - LED RGB catodo comum, so 2 cores: D32 e D33 (comum no GND)
 *    - OLED SSD1306 I2C: SDA=D25, SCL=D26 (remapeado por software)
 *    - Buzzer em D13
 *    - SEM botoes / SEM interruptor fisico: "ligado" vem de um limiar do pot
 *
 *  COMPATIBILIDADE PWM: o core ESP32 3.x trocou a API de LEDC.
 *  Usamos a API NOVA (ledcAttach / ledcWrite por PINO), que funciona no 3.x.
 * ============================================================================
 */

#include "config.h"
#include "ITransporte.h"
#include "TransporteHTTP.h"
#include "FiltroMediaMovel.h"

// ---- Display OLED ----
Adafruit_SSD1306 display(LARGURA, ALTURA, &Wire, -1);

// ---- Transporte: troque SO esta linha para mudar a forma de envio ----
ITransporte* transporte = new TransporteHTTP(WIFI_SSID, WIFI_SENHA, URL_DESTINO);

// ---- Filtros (um para cada sensor analogico) ----
FiltroMediaMovel<TAMANHO_JANELA> filtroEnergia;
FiltroMediaMovel<TAMANHO_JANELA> filtroSom;

// ---- Controle de tempo (millis, nao-bloqueante) ----
unsigned long ultimaLeitura = 0;
unsigned long ultimoEnvio = 0;
unsigned long fimDoBip = 0;   // marca quando o bip deve parar (millis)

// ---- Estado atual lido dos sensores ----
float energiaFiltrada = 0;
float somFiltrado = 0;
bool ligado = false;

// ---- Gamificacao: pontuacao simples ----
long pontuacao = 0;

// ---- Prototipos ----
void enviarTelemetria();
void atualizarLed();
void atualizarTela();
void bipCurto();

void setup() {
  Serial.begin(115200);
  delay(300); // unico delay aceitavel: so no boot

  // --- PWM (LEDC) para o LED RGB e o buzzer (API nova do core 3.x) ---
  // ledcAttach(pino, frequencia, resolucao_em_bits)
  ledcAttach(PINO_LED_A, 5000, 8);
  ledcAttach(PINO_LED_B, 5000, 8);
  ledcAttach(PINO_BUZZER, 2000, 8);
  ledcWrite(PINO_BUZZER, 0);   // buzzer silenciado no boot

  // --- OLED ---
  Wire.begin(PINO_SDA, PINO_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR)) {
    Serial.println("[BOOT] Falha ao iniciar o OLED");
  } else {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Iniciando...");
    display.display();
  }

  Serial.printf("[BOOT] Transporte ativo: %s\n", transporte->nome());
  transporte->iniciar();
}

void loop() {
  unsigned long agora = millis();

  // mantem a conexao viva (reconecta se cair) - nao-bloqueante
  transporte->manter();

  // desliga o bip quando o tempo dele acabar (sem travar o loop)
  if (fimDoBip != 0 && agora >= fimDoBip) {
    ledcWrite(PINO_BUZZER, 0);
    fimDoBip = 0;
  }

  // ---- Tarefa 1: ler sensores periodicamente ----
  if (agora - ultimaLeitura >= INTERVALO_LEITURA) {
    ultimaLeitura = agora;

    int leituraEnergia = analogRead(PINO_POT_ENERGIA);
    int leituraSom = analogRead(PINO_POT_SOM);

    // filtro de media movel (obrigatorio)
    energiaFiltrada = filtroEnergia.adicionar(leituraEnergia);
    somFiltrado = filtroSom.adicionar(leituraSom);

    // Sem interruptor: "ligado" quando o pot de energia passa do limiar
    ligado = (energiaFiltrada > LIMIAR_LIGADO);

    atualizarLed();
    atualizarTela();
  }

  // ---- Tarefa 2: enviar telemetria periodicamente ----
  if (agora - ultimoEnvio >= INTERVALO_ENVIO) {
    ultimoEnvio = agora;
    enviarTelemetria();
  }
}

// Monta o JSON e manda pela interface (sem saber se e HTTP, MQTT, etc.)
void enviarTelemetria() {
  String json = "{";
  json += "\"energia\":" + String(energiaFiltrada, 1) + ",";
  json += "\"som\":" + String(somFiltrado, 1) + ",";
  json += "\"ligado\":" + String(ligado ? "true" : "false") + ",";
  json += "\"pontuacao\":" + String(pontuacao) + ",";
  json += "\"uptime_ms\":" + String(millis());
  json += "}";

  bool ok = transporte->enviar(json);
  if (ok) {
    pontuacao++;
    bipCurto();
  }
}

// LED RGB de 2 cores: mistura conforme o nivel de "energia" filtrado.
// Catodo comum -> a cor acende quando o PWM sobe (HIGH).
void atualizarLed() {
  if (!ligado) {
    ledcWrite(PINO_LED_A, 0);   // vermelho
    ledcWrite(PINO_LED_B, 0);   // verde
    return;
  }

  // energia filtrada vai de ~0 a 4095
  if (energiaFiltrada < 1500) {
    // FRACO -> vermelho
    ledcWrite(PINO_LED_A, 255);   // vermelho no maximo
    ledcWrite(PINO_LED_B, 0);     // verde apagado
  } else if (energiaFiltrada < 2800) {
    // MEDIO -> amarelo (vermelho + verde juntos)
    ledcWrite(PINO_LED_A, 255);   // vermelho
    ledcWrite(PINO_LED_B, 255);   // verde
  } else {
    // FORTE -> verde
    ledcWrite(PINO_LED_A, 0);     // vermelho apagado
    ledcWrite(PINO_LED_B, 255);   // verde no maximo
  }
}

// Mostra energia, som, estado e pontuacao na tela OLED.
void atualizarTela() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Estado: ");
  display.println(ligado ? "LIGADO" : "desligado");

  display.setTextSize(2);
  display.setCursor(0, 12);
  display.print("E:");
  display.println((int)energiaFiltrada);

  display.setCursor(0, 32);
  display.print("S:");
  display.println((int)somFiltrado);

  display.setTextSize(1);
  display.setCursor(0, 54);
  display.print("Pontos: ");
  display.print(pontuacao);
  display.print(transporte->conectado() ? "  WiFi:ok" : "  WiFi:--");

  display.display();
}

// Bip curto NAO-bloqueante: liga o som e agenda o desligamento p/ 60ms depois.
void bipCurto() {
  ledcWriteTone(PINO_BUZZER, 1200); // liga som a 1200 Hz
  fimDoBip = millis() + 60;         // sera desligado daqui a 60 ms
}
