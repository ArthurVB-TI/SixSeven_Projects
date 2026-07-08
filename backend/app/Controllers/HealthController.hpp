#pragma once
// ============================================================
//  app/Controllers/HealthController.hpp
//  Rota simples de saude do servico (sem autenticacao). Util
//  para monitoramento e para o front checar se a API esta no ar.
//
//  GET /health -> {ok:true, data:{status:"up", app:...}}
// ============================================================

#include <drogon/HttpController.h>
#include "BaseController.hpp"
#include "../Config/AppConfig.hpp"

namespace app::controllers {

class HealthController : public drogon::HttpController<HealthController>,
                         public BaseController {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(HealthController::health, "/health", drogon::Get, drogon::Options);
    METHOD_LIST_END

    void health(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& cb) {
        handle([&]() {
            Json::Value j;
            j["status"] = "up";
            j["app"] = "SixSeven_Projects";
            j["env"] = config::AppConfig::get().appEnv();
            return helpers::Response::success(j);
        }, std::move(cb));
    }
};

} // namespace app::controllers
