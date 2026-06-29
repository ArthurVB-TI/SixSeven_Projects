#ifndef TRANSPORTE_HTTP_H
#define TRANSPORTE_HTTP_H

#include "ITransporte.h"
#include <WiFi.h>
#include <HTTPClient.h>

/*
 * TransporteHTTP — implementacao concreta de ITransporte que envia os dados
 * via Wi-Fi usando requisicoes HTTP POST.
 *
 * Esta é a UNICA parte do projeto que conhece Wi-Fi e HTTP.
 * Se um dia voce quiser MQTT, crie TransporteMQTT.h com a mesma "cara"
 * (os mesmos metodos publicos) e troque so a linha que cria o objeto no main.
 */
class TransporteHTTP : public ITransporte {
  private:
    const char* _ssid;
    const char* _senha;
    String _url;            // endpoint que recebe o POST (ex: http://192.168.0.10:3000/dados)
    unsigned long _ultimaTentativa = 0;
    const unsigned long _intervaloReconexao = 5000; // tenta religar Wi-Fi a cada 5s

  public:
    TransporteHTTP(const char* ssid, const char* senha, const String& url)
      : _ssid(ssid), _senha(senha), _url(url) {}

    bool iniciar() override {
      WiFi.mode(WIFI_STA);
      WiFi.begin(_ssid, _senha);
      Serial.print("[HTTP] Conectando ao Wi-Fi");
      unsigned long inicio = millis();
      // espera ate 10s pela conexao (so no boot; depois é tudo nao-bloqueante)
      while (WiFi.status() != WL_CONNECTED && millis() - inicio < 10000) {
        delay(250);
        Serial.print(".");
      }
      Serial.println();
      if (WiFi.status() == WL_CONNECTED) {
        Serial.print("[HTTP] Conectado! IP: ");
        Serial.println(WiFi.localIP());
        return true;
      }
      Serial.println("[HTTP] Falha ao conectar (vai tentar de novo no loop).");
      return false;
    }

    bool enviar(const String& payload) override {
      if (!conectado()) return false;

      HTTPClient http;
      http.begin(_url);
      http.addHeader("Content-Type", "application/json");
      int codigo = http.POST(payload);
      http.end();

      if (codigo > 0) {
        Serial.printf("[HTTP] POST enviado, resposta: %d\n", codigo);
        return (codigo >= 200 && codigo < 300);
      } else {
        Serial.printf("[HTTP] Erro no POST: %s\n", http.errorToString(codigo).c_str());
        return false;
      }
    }

    bool conectado() override {
      return WiFi.status() == WL_CONNECTED;
    }

    // Nao-bloqueante: se o Wi-Fi caiu, tenta reconectar sem travar o resto.
    void manter() override {
      if (conectado()) return;
      if (millis() - _ultimaTentativa >= _intervaloReconexao) {
        _ultimaTentativa = millis();
        Serial.println("[HTTP] Wi-Fi caiu, tentando reconectar...");
        WiFi.disconnect();
        WiFi.begin(_ssid, _senha);
      }
    }

    const char* nome() override { return "HTTP"; }
};

#endif
