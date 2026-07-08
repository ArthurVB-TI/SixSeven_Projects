#pragma once
// ============================================================
//  app/Controllers/ConectionController.hpp
//  Rotas das conexoes (hardwares) do usuario logado. Todas
//  protegidas por AuthMiddleware; o ConectionService valida a
//  posse (um usuario so mexe nas proprias conexoes).
//
//  GET    /conections          -> lista as conexoes do usuario
//  POST   /conections          -> cria conexao
//  GET    /conections/{id}     -> detalhe de uma conexao
//  PUT    /conections/{id}     -> atualiza conexao
//  DELETE /conections/{id}     -> remove conexao
// ============================================================

#include <drogon/HttpController.h>
#include "BaseController.hpp"
#include "../Service/ConectionService.hpp"
#include "../Helpers/RequestParser.hpp"
#include "../Security/Sanitizer.hpp"

namespace app::controllers {

class ConectionController : public drogon::HttpController<ConectionController>,
                            public BaseController {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(ConectionController::list,   "/conections", drogon::Get, drogon::Options,
                  "app::middleware::AuthMiddleware");
    ADD_METHOD_TO(ConectionController::create, "/conections", drogon::Post, drogon::Options,
                  "app::middleware::AuthMiddleware");
    ADD_METHOD_TO(ConectionController::detail, "/conections/{1}", drogon::Get, drogon::Options,
                  "app::middleware::AuthMiddleware");
    ADD_METHOD_TO(ConectionController::update, "/conections/{1}", drogon::Put, drogon::Options,
                  "app::middleware::AuthMiddleware");
    ADD_METHOD_TO(ConectionController::remove, "/conections/{1}", drogon::Delete, drogon::Options,
                  "app::middleware::AuthMiddleware");
    METHOD_LIST_END

    void list(const drogon::HttpRequestPtr& req,
              std::function<void(const drogon::HttpResponsePtr&)>&& cb) {
        handle([&]() {
            return helpers::Response::success(service_.listByUsuario(uid(req)));
        }, std::move(cb));
    }

    void create(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& cb) {
        handle([&]() {
            helpers::RequestParser p(req);
            int id = service_.create(p.requireString("nome"),
                                     p.optionalBool("can_active", false),
                                     uid(req));
            Json::Value j;
            j["id"] = id;
            return helpers::Response::success(j, drogon::k201Created);
        }, std::move(cb));
    }

    void detail(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& cb,
                const std::string& idStr) {
        handle([&]() {
            int id = security::Sanitizer::toPositiveInt(idStr, "id");
            return helpers::Response::success(service_.getOwned(id, uid(req)));
        }, std::move(cb));
    }

    void update(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& cb,
                const std::string& idStr) {
        handle([&]() {
            int id = security::Sanitizer::toPositiveInt(idStr, "id");
            helpers::RequestParser p(req);
            service_.update(id, uid(req),
                            p.requireString("nome"),
                            p.optionalBool("can_active", false),
                            p.optionalBool("is_active", false));
            return helpers::Response::success();
        }, std::move(cb));
    }

    void remove(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& cb,
                const std::string& idStr) {
        handle([&]() {
            int id = security::Sanitizer::toPositiveInt(idStr, "id");
            service_.remove(id, uid(req));
            return helpers::Response::success();
        }, std::move(cb));
    }

private:
    service::ConectionService service_;
};

} // namespace app::controllers
