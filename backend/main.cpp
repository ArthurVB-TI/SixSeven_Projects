// ============================================================
//  main.cpp  -  Ponto de entrada do backend SixSeven_Projects
//
//  Fluxo:
//   1) Carrega a config de infraestrutura (config/app.config.json):
//      listeners, threads, log, db_clients, e o bloco "security".
//   2) Roda o Bootstrap (registra filtros e o 404 padronizado;
//      os controllers se auto-registram ao serem incluidos).
//   3) Sobe o event loop do Drogon (servidor HTTP assincrono).
//
//  Compilacao: ver composers/ (CMakeLists.txt) e o README.
// ============================================================

#include <drogon/drogon.h>
#include "bootstrap/Bootstrap.hpp"

int main() {
    // 1) Carrega toda a configuracao a partir do arquivo JSON.
    //    Tudo que e infraestrutura fica la; o codigo le via
    //    AppConfig (bloco "security") e via Drogon (o resto).
    drogon::app().loadConfigFile("config/app.config.json");

    // 2) Inicializacao central (filtros globais, 404, etc.).
    app::bootstrap::Bootstrap::init();

    LOG_INFO << "SixSeven_Projects backend iniciando...";

    // 3) Sobe o servidor (bloqueante ate o encerramento).
    drogon::app().run();

    return 0;
}
