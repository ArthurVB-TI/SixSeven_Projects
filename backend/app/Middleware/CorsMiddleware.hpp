#pragma once
// ============================================================
//  app/Middleware/CorsMiddleware.hpp
//  Filtro global de CORS. Responde requisicoes OPTIONS
//  (preflight) imediatamente e injeta os cabecalhos nas demais
//  respostas via o helper Response (aplicado nos controllers).
//  Registrado como filtro no roteamento.
// ============================================================

#include <drogon/HttpFilter.h>
#include "../Helpers/Response.hpp"

namespace app::middleware {

// AutoCreation=false: este filtro e registrado manualmente como
// filtro global em Bootstrap::init(); o Drogon proibe registro
// manual de filtros auto-criados.
class CorsMiddleware : public drogon::HttpFilter<CorsMiddleware, false> {
public:
    void doFilter(const drogon::HttpRequestPtr& req,
                  drogon::FilterCallback&& fcb,
                  drogon::FilterChainCallback&& fccb) override {
        if (req->method() == drogon::Options) {
            auto resp = drogon::HttpResponse::newHttpResponse();
            resp->setStatusCode(drogon::k204NoContent);
            helpers::Response::applyCors(resp);
            fcb(resp);
            return;
        }
        fccb(); // segue a cadeia
    }
};

} // namespace app::middleware
