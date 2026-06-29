#ifndef ITRANSPORTE_H
#define ITRANSPORTE_H

#include <Arduino.h>

/*
 * ITransporte — Interface (classe abstrata) que define COMO os dados saem do ESP32.
 *
 * A ideia central: o resto do firmware (leitura de sensores, filtro, gamificacao)
 * NUNCA fala com Wi-Fi/HTTP diretamente. Ele so chama metodos desta interface.
 *
 * Para trocar HTTP por MQTT, WebSocket, Bluetooth, etc. no futuro, voce cria
 * uma nova classe que herda de ITransporte e implementa estes metodos.
 * NADA mais no projeto precisa mudar.
 *
 * Isso é o "Strategy Pattern": cada forma de envio é uma estrategia intercambiavel.
 */
class ITransporte {
  public:
    virtual ~ITransporte() {}

    // Inicializa a conexao (conectar no Wi-Fi, abrir socket, parear BT...).
    // Retorna true se conseguiu se conectar.
    virtual bool iniciar() = 0;

    // Envia um pacote de dados (geralmente em JSON). Retorna true se enviou OK.
    virtual bool enviar(const String& payload) = 0;

    // Diz se a conexao esta ativa no momento.
    virtual bool conectado() = 0;

    // Tenta reconectar caso a conexao tenha caido. Chamado de tempos em tempos.
    virtual void manter() = 0;

    // Nome legivel do transporte (para log/debug). Ex: "HTTP", "MQTT".
    virtual const char* nome() = 0;
};

#endif
