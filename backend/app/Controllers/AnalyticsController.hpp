#pragma once
// ============================================================
//  app/Controllers/AnalyticsController.hpp
//  Rotas de graficos/metricas para a pagina de informacoes.
//  Protegidas por AuthMiddleware; valida a posse da conexao.
//
//  GET /analytics/{id}/dashboard?limite=N
//        -> todos os graficos (Strategy) + resumo estatistico
//  GET /analytics/charts
//        -> lista os graficos disponiveis (para o front montar abas)
// ============================================================

#include <drogon/HttpController.h>
#include "BaseController.hpp"
#include "../Service/AnalyticsService.hpp"
#include "../Service/ConectionService.hpp"
#include "../Security/Sanitizer.hpp"

namespace app::controllers {

class AnalyticsController : public drogon::HttpController<AnalyticsController>,
                            public BaseController {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(AnalyticsController::dashboard, "/analytics/{1}/dashboard",
                  drogon::Get, drogon::Options, "app::middleware::AuthMiddleware");
    ADD_METHOD_TO(AnalyticsController::charts, "/analytics/charts",
                  drogon::Get, drogon::Options, "app::middleware::AuthMiddleware");
    METHOD_LIST_END

    void dashboard(const drogon::HttpRequestPtr& req,
                   std::function<void(const drogon::HttpResponsePtr&)>&& cb,
                   const std::string& idStr) {
        handle([&]() {
            int id = security::Sanitizer::toPositiveInt(idStr, "id");
            conections_.requireOwned(id, uid(req));

            int limite = 50; // padrao
            std::string limStr = req->getParameter("limite");
            if (!limStr.empty()) {
                limite = security::Sanitizer::toPositiveInt(limStr, "limite");
                if (limite > 500) limite = 500; // teto de seguranca
            }
            return helpers::Response::success(analytics_.dashboard(id, limite));
        }, std::move(cb));
    }

    void charts(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& cb) {
        handle([&]() {
            return helpers::Response::success(analytics_.availableCharts());
        }, std::move(cb));
    }

private:
    service::AnalyticsService analytics_;
    service::ConectionService conections_;
};

} // namespace app::controllers
