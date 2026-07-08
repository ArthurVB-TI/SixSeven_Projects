#pragma once
// ============================================================
//  app/Helpers/Response.hpp
//  Fabrica de respostas JSON padronizadas. Todo endpoint
//  responde no formato { "ok": bool, "data": ..., "error": ... }.
//  Mantem o CORS coerente com a config.
// ============================================================

#include <drogon/HttpResponse.h>
#include <json/json.h>
#include "../Config/AppConfig.hpp"

namespace app::helpers {

class Response {
public:
    static drogon::HttpResponsePtr success(const Json::Value& data,
                                           drogon::HttpStatusCode code = drogon::k200OK) {
        Json::Value body;
        body["ok"] = true;
        body["data"] = data;
        return make(body, code);
    }

    static drogon::HttpResponsePtr success(drogon::HttpStatusCode code = drogon::k200OK) {
        Json::Value body;
        body["ok"] = true;
        body["data"] = Json::Value(Json::nullValue);
        return make(body, code);
    }

    static drogon::HttpResponsePtr error(const std::string& message,
                                         drogon::HttpStatusCode code) {
        Json::Value body;
        body["ok"] = false;
        body["error"] = message;
        return make(body, code);
    }

    // Aplica os cabecalhos CORS. Reutilizado pelo middleware para OPTIONS.
    static void applyCors(const drogon::HttpResponsePtr& resp) {
        const auto& origin = app::config::AppConfig::get().corsAllowOrigin();
        resp->addHeader("Access-Control-Allow-Origin", origin);
        resp->addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        resp->addHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
        resp->addHeader("Access-Control-Allow-Credentials", "true");
    }

private:
    static drogon::HttpResponsePtr make(const Json::Value& body,
                                        drogon::HttpStatusCode code) {
        auto resp = drogon::HttpResponse::newHttpJsonResponse(body);
        resp->setStatusCode(code);
        applyCors(resp);
        return resp;
    }
};

} // namespace app::helpers
