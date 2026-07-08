#pragma once
// ============================================================
//  app/Controllers/HardwareController.hpp
//  A ponte HTTP com o hardware. Tem DOIS publicos distintos:
//
//  (A) O FIRMWARE (ESP32) - contrato IDENTICO ao codigo do
//      hardware, NAO alterar. Sem autenticacao e SEM envelope
//      {ok,data}: o firmware espera o JSON cru.
//        POST /data           corpo: {id,E_b,E_r,timer,M_er,V_mer,M_vmer}
//        GET  /config?id=N    resposta: {id,E_b,timer}
//
//  (B) O SITE (React) - autenticado (AuthMiddleware) e com o
//      envelope padrao. Usa a posse validada pela conexao.
//        GET  /hardware/{id}/info     -> ultima leitura + derivadas
//        POST /hardware/{id}/config   -> grava E_b/timer (manipular)
//
//  IMPORTANTE: as rotas do firmware respondem com
//  newHttpJsonResponse direto (sem passar pelo helper de
//  envelope), justamente para manter o shape que o ESP32 le.
// ============================================================

#include <drogon/HttpController.h>
#include "BaseController.hpp"
#include "../Service/HardwareService.hpp"
#include "../Service/ConectionService.hpp"
#include "../Helpers/RequestParser.hpp"
#include "../Security/Sanitizer.hpp"

namespace app::controllers {

class HardwareController : public drogon::HttpController<HardwareController>,
                           public BaseController {
public:
    METHOD_LIST_BEGIN
    // ---- (A) Rotas do firmware: SEM AuthMiddleware, shape cru ----
    ADD_METHOD_TO(HardwareController::hwPush,   "/data",   drogon::Post, drogon::Options);
    ADD_METHOD_TO(HardwareController::hwConfig, "/config", drogon::Get,  drogon::Options);

    // ---- (B) Rotas do site: protegidas, com envelope ----
    ADD_METHOD_TO(HardwareController::siteInfo,      "/hardware/{1}/info",
                  drogon::Get, drogon::Options, "app::middleware::AuthMiddleware");
    ADD_METHOD_TO(HardwareController::siteSetConfig, "/hardware/{1}/config",
                  drogon::Post, drogon::Options, "app::middleware::AuthMiddleware");
    METHOD_LIST_END

    // ================= (A) FIRMWARE =================

    // POST /data  <- hardware envia leitura.
    // Responde {ok:true} cru (o firmware ignora o corpo, mas
    // mantemos algo simples e sem o envelope do site).
    void hwPush(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& cb) {
        try {
            auto json = req->getJsonObject();
            if (!json) {
                auto r = drogon::HttpResponse::newHttpResponse();
                r->setStatusCode(drogon::k400BadRequest);
                cb(r);
                return;
            }
            hw_.ingestPush(*json);

            Json::Value out;
            out["ok"] = true;
            auto r = drogon::HttpResponse::newHttpJsonResponse(out);
            cb(r);
        } catch (const std::exception& e) {
            LOG_ERROR << "hwPush: " << e.what();
            // Mesmo em erro, resposta simples (sem envelope do site).
            Json::Value out; out["ok"] = false;
            auto r = drogon::HttpResponse::newHttpJsonResponse(out);
            r->setStatusCode(drogon::k400BadRequest);
            cb(r);
        }
    }

    // GET /config?id=N  -> {id,E_b,timer} exatamente como o firmware le.
    void hwConfig(const drogon::HttpRequestPtr& req,
                  std::function<void(const drogon::HttpResponsePtr&)>&& cb) {
        try {
            std::string idStr = req->getParameter("id");
            int id = security::Sanitizer::toPositiveInt(idStr, "id");
            Json::Value cfg = hw_.buildConfig(id); // {id,E_b,timer}
            auto r = drogon::HttpResponse::newHttpJsonResponse(cfg);
            cb(r);
        } catch (const std::exception& e) {
            LOG_ERROR << "hwConfig: " << e.what();
            auto r = drogon::HttpResponse::newHttpResponse();
            r->setStatusCode(drogon::k400BadRequest);
            cb(r);
        }
    }

    // ================= (B) SITE =================

    // GET /hardware/{id}/info  -> ultima leitura detalhada.
    void siteInfo(const drogon::HttpRequestPtr& req,
                  std::function<void(const drogon::HttpResponsePtr&)>&& cb,
                  const std::string& idStr) {
        handle([&]() {
            int id = security::Sanitizer::toPositiveInt(idStr, "id");
            // valida posse antes de expor os dados:
            conections_.requireOwned(id, uid(req));
            return helpers::Response::success(hw_.currentInfo(id));
        }, std::move(cb));
    }

    // POST /hardware/{id}/config  -> grava E_b/timer (manipular hardware).
    void siteSetConfig(const drogon::HttpRequestPtr& req,
                       std::function<void(const drogon::HttpResponsePtr&)>&& cb,
                       const std::string& idStr) {
        handle([&]() {
            int id = security::Sanitizer::toPositiveInt(idStr, "id");
            conections_.requireOwned(id, uid(req));
            helpers::RequestParser p(req);
            hw_.setConfig(id, p.requireInt("E_b"), p.requireInt("timer"));
            return helpers::Response::success();
        }, std::move(cb));
    }

private:
    service::HardwareService hw_;
    service::ConectionService conections_;
};

} // namespace app::controllers
