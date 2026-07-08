#pragma once
// ============================================================
//  bootstrap/Bootstrap.hpp
//  Inicializacao central da aplicacao. No Drogon, controllers e
//  filtros se auto-registram quando suas classes sao "linkadas"
//  ao binario; incluir os headers aqui garante que o compilador
//  os inclua na traducao. Este arquivo tambem centraliza ajustes
//  programaticos que nao ficam no JSON (ex.: handlers globais).
//
//  main.cpp inclui este header e chama Bootstrap::init() antes de
//  subir o servidor.
// ============================================================

#include <drogon/drogon.h>

// --- Filtros (middleware) ---
#include "../app/Middleware/CorsMiddleware.hpp"
#include "../app/Middleware/AuthMiddleware.hpp"

// --- Controllers (auto-registram as rotas via METHOD_LIST) ---
#include "../app/Controllers/HealthController.hpp"
#include "../app/Controllers/AuthController.hpp"
#include "../app/Controllers/ConectionController.hpp"
#include "../app/Controllers/DispositivoController.hpp"
#include "../app/Controllers/HardwareController.hpp"
#include "../app/Controllers/AnalyticsController.hpp"

namespace app::bootstrap {

class Bootstrap {
public:
    static void init() {
        // CORS global: aplica o filtro a todas as rotas. Ele trata
        // o preflight OPTIONS e injeta os cabecalhos. As respostas
        // "normais" ja recebem CORS pelo helper Response.
        drogon::app().registerFilter(
            std::make_shared<app::middleware::CorsMiddleware>());

        // Handler 404 padronizado (mesmo envelope do resto da API).
        drogon::app().setCustom404Page(
            app::helpers::Response::error("Rota nao encontrada",
                                          drogon::k404NotFound));

        LOG_INFO << "Bootstrap concluido: filtros e controllers registrados.";
    }
};

} // namespace app::bootstrap
