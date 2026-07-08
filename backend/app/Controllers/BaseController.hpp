#pragma once
// ============================================================
//  app/Controllers/BaseController.hpp
//  Utilitario compartilhado pelos controllers. O metodo handle()
//  executa a logica e converte QUALQUER excecao de dominio
//  (ApiException) ou inesperada em uma resposta JSON padronizada.
//  Centraliza o tratamento de erros (o controller so escreve o
//  "caminho feliz").
// ============================================================

#include <functional>
#include <drogon/HttpController.h>
#include "../Helpers/Response.hpp"
#include "../Exceptions/ApiException.hpp"

namespace app::controllers {

class BaseController {
protected:
    // Executa 'fn' e responde. fn retorna um HttpResponsePtr.
    static void handle(std::function<drogon::HttpResponsePtr()> fn,
                       std::function<void(const drogon::HttpResponsePtr&)>&& callback) {
        try {
            callback(fn());
        } catch (const exceptions::ApiException& e) {
            callback(helpers::Response::error(e.message(), e.status()));
        } catch (const drogon::orm::DrogonDbException& e) {
            // Erro vindo do banco (constraint, procedure, etc.)
            LOG_ERROR << "DB error: " << e.base().what();
            callback(helpers::Response::error("Erro de banco de dados",
                                              drogon::k500InternalServerError));
        } catch (const std::exception& e) {
            LOG_ERROR << "Unhandled: " << e.what();
            callback(helpers::Response::error("Erro interno",
                                              drogon::k500InternalServerError));
        }
    }

    // Le o uid injetado pelo AuthMiddleware.
    static int uid(const drogon::HttpRequestPtr& req) {
        return req->attributes()->get<int>("uid");
    }
};

} // namespace app::controllers
