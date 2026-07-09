/*
 * ============================================================================
 *  PROJETO INTEGRADOR I - Parte Hardware (ESP32-S3)
 *  Le sensores, mostra no OLED e envia para o backend SixSeven.
 * ============================================================================
 *
 *  FLUXO:
 *    - Pot de ENERGIA (PINO_POT_ENERGIA) -> E_r (energia recebida / leitura atual)
 *    - Pot de SOM     (PINO_POT_SOM)     -> E_b (energia base / referencia)
 *    - Filtro de media movel calcula M_er (media de E_r) e M_vmer (media da variacao)
 *    - V_mer = |E_r - M_er|  (o backend considera "estavel" quando V_mer <= 150)
 *    - OLED mostra E_r, E_b, estabilidade e status de Wi-Fi
 *    - POST no backend em /data no formato: {id,E_b,E_r,timer,M_er,V_mer,M_vmer}
 *
 *  Pinos e credenciais ficam no config.h / secrets.h (nao alterados aqui).
 *
 *  COMPATIBILIDADE PWM: core ESP32 3.x usa a API NOVA (ledcAttach/ledcWrite
 *  por PINO), adotada abaixo.
 * ============================================================================
 */

#include "config.h"
#include "ITransporte.h"
#include "TransporteHTTP.hpp"
#include "FiltroMediaMovel.h"

// ---- Identidade/limiar deste hardware (nao mexe no config.h) ----
static const int CONEXAO_ID     = 1;    // id desta conexao no backend
static const int LIMIAR_ESTAVEL = 150;  // V_mer <= isso => estavel (igual ao backend)

// ---- Display OLED ----
Adafruit_SSD1306 display(OLED_LARGURA, OLED_ALTURA, &Wire, -1);

// ---- Transporte: troque SO esta linha para mudar a forma de envio ----
ITransporte* transporte = new TransporteHTTP(WIFI_SSID, WIFI_SENHA, URL_DESTINO);

// ---- Filtros de media movel (buffer circular) ----
FiltroMediaMovel<TAMANHO_JANELA> filtroEnergia;   // E_r  -> M_er
FiltroMediaMovel<TAMANHO_JANELA> filtroBase;      // pot2 -> E_b (suavizado)
FiltroMediaMovel<TAMANHO_JANELA> filtroVariacao;  // V_mer -> M_vmer

// ---- Controle de tempo (millis, nao-bloqueante) ----
unsigned long ultimaLeitura = 0;
unsigned long ultimoEnvio = 0;
unsigned long fimDoBip = 0;   // marca quando o bip deve parar (millis)

// ---- Grandezas no formato do backend ----
int E_r = 0;      // energia recebida (leitura crua do pot de energia)
int M_er = 0;     // media movel de E_r
int V_mer = 0;    // variacao = |E_r - M_er|
int M_vmer = 0;   // media movel da variacao
int E_b = 0;      // energia base (pot de "som" reaproveitado como referencia)
bool ligado = false;

// ---- Contador de envios com sucesso (feedback no OLED) ----
long envios = 0;

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
  Wire.begin(PINO_OLED_SDA, PINO_OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ENDERECO)) {
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
    int leituraBase    = analogRead(PINO_POT_SOM);   // 2o pot = energia base

    // E_r cru + medias moveis (filtro obrigatorio)
    E_r    = leituraEnergia;
    M_er   = (int) filtroEnergia.adicionar(leituraEnergia);
    E_b    = (int) filtroBase.adicionar(leituraBase);
    V_mer  = abs(E_r - M_er);
    M_vmer = (int) filtroVariacao.adicionar(V_mer);

    // Sem interruptor: "ligado" quando o pot de energia passa do limiar
    ligado = (E_r > LIMIAR_LIGADO);

    atualizarLed();
    atualizarTela();
  }

  // ---- Tarefa 2: enviar telemetria periodicamente ----
  if (agora - ultimoEnvio >= INTERVALO_ENVIO) {
    ultimoEnvio = agora;
    enviarTelemetria();
  }
}

// Monta o JSON no formato do backend e manda pela interface (HTTP, MQTT, etc.).
// Contrato do backend (POST /data): {id,E_b,E_r,timer,M_er,V_mer,M_vmer}
void enviarTelemetria() {
  String json = "{";
  json += "\"id\":"     + String(CONEXAO_ID) + ",";
  json += "\"E_b\":"    + String(E_b) + ",";
  json += "\"E_r\":"    + String(E_r) + ",";
  json += "\"timer\":"  + String(INTERVALO_ENVIO) + ",";
  json += "\"M_er\":"   + String(M_er) + ",";
  json += "\"V_mer\":"  + String(V_mer) + ",";
  json += "\"M_vmer\":" + String(M_vmer);
  json += "}";

  bool ok = transporte->enviar(json);
  if (ok) {
    envios++;
    bipCurto();
  }
}

// LED RGB de 2 cores: mistura conforme o nivel de energia (media).
// Catodo comum -> a cor acende quando o PWM sobe (HIGH).
void atualizarLed() {
  if (!ligado) {
    ledcWrite(PINO_LED_A, 0);   // vermelho
    ledcWrite(PINO_LED_B, 0);   // verde
    return;
  }

  // M_er vai de ~0 a 4095
  if (M_er < 1500) {
    // FRACO -> vermelho
    ledcWrite(PINO_LED_A, 255);
    ledcWrite(PINO_LED_B, 0);
  } else if (M_er < 2800) {
    // MEDIO -> amarelo (vermelho + verde)
    ledcWrite(PINO_LED_A, 255);
    ledcWrite(PINO_LED_B, 255);
  } else {
    // FORTE -> verde
    ledcWrite(PINO_LED_A, 0);
    ledcWrite(PINO_LED_B, 255);
  }
}

// Mostra E_r, E_b, estabilidade e Wi-Fi no OLED.
void atualizarTela() {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  bool estavel = (V_mer <= LIMIAR_ESTAVEL);

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Estado: ");
  display.println(estavel ? "ESTAVEL" : "instavel");

  display.setTextSize(2);
  display.setCursor(0, 12);
  display.print("Er:");
  display.println(E_r);

  display.setTextSize(1);
  display.setCursor(0, 34);
  display.print("Eb:");
  display.print(E_b);
  display.print("  V:");
  display.println(V_mer);

  display.setCursor(0, 54);
  display.print("Env:");
  display.print(envios);
  display.print(transporte->conectado() ? "  WiFi:ok" : "  WiFi:--");

  display.display();
}

// Bip curto NAO-bloqueante: liga o som e agenda o desligamento p/ 60ms depois.
void bipCurto() {
  ledcWriteTone(PINO_BUZZER, 1200); // liga som a 1200 Hz
  fimDoBip = millis() + 60;         // sera desligado daqui a 60 ms
}
