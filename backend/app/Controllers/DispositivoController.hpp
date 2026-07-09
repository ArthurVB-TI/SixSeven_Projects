#pragma once
// ============================================================
//  app/Controllers/DispositivoController.hpp
//  Rotas dos dispositivos e da relacao N:N com conexoes.
//  Todas protegidas por AuthMiddleware.
//
//  GET    /dispositivos                      -> lista dispositivos
//  POST   /dispositivos                      -> cria dispositivo
//  DELETE /dispositivos/{id}                 -> remove dispositivo
//  GET    /dispositivos/links                -> lista relacoes N:N
//  POST   /dispositivos/link                 -> associa disp. a conexao
//  POST   /dispositivos/unlink               -> desassocia
// ============================================================

#include <drogon/HttpController.h>
#include "BaseController.hpp"
#include "../Service/DispositivoService.hpp"
#include "../Helpers/RequestParser.hpp"
#include "../Security/Sanitizer.hpp"

namespace app::controllers {

class DispositivoController : public drogon::HttpController<DispositivoController>,
                              public BaseController {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(DispositivoController::list,   "/dispositivos", drogon::Get, drogon::Options,
                  "app::middleware::AuthMiddleware");
    ADD_METHOD_TO(DispositivoController::create, "/dispositivos", drogon::Post, drogon::Options,
                  "app::middleware::AuthMiddleware");
    ADD_METHOD_TO(DispositivoController::links,  "/dispositivos/links", drogon::Get, drogon::Options,
                  "app::middleware::AuthMiddleware");
    ADD_METHOD_TO(DispositivoController::link,   "/dispositivos/link", drogon::Post, drogon::Options,
                  "app::middleware::AuthMiddleware");
    ADD_METHOD_TO(DispositivoController::unlink, "/dispositivos/unlink", drogon::Post, drogon::Options,
                  "app::middleware::AuthMiddleware");
    ADD_METHOD_TO(DispositivoController::remove, "/dispositivos/{1}", drogon::Delete, drogon::Options,
                  "app::middleware::AuthMiddleware");
    METHOD_LIST_END

    void list(const drogon::HttpRequestPtr& req,
              std::function<void(const drogon::HttpResponsePtr&)>&& cb) {
        handle([&]() {
            return helpers::Response::success(service_.list(uid(req)));
        }, std::move(cb));
    }

    void create(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& cb) {
        handle([&]() {
            helpers::RequestParser p(req);
            int id = service_.create(p.requireString("nome"),
                                     p.requireString("tipo"),
                                     uid(req));
            Json::Value j; j["id"] = id;
            return helpers::Response::success(j, drogon::k201Created);
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

    void links(const drogon::HttpRequestPtr& req,
               std::function<void(const drogon::HttpResponsePtr&)>&& cb) {
        handle([&]() {
            return helpers::Response::success(service_.links(uid(req)));
        }, std::move(cb));
    }

    void link(const drogon::HttpRequestPtr& req,
              std::function<void(const drogon::HttpResponsePtr&)>&& cb) {
        handle([&]() {
            helpers::RequestParser p(req);
            service_.link(p.requireInt("id_conection"),
                          p.requireInt("id_dispositivo"),
                          uid(req));
            return helpers::Response::success(drogon::k201Created);
        }, std::move(cb));
    }

    void unlink(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& cb) {
        handle([&]() {
            helpers::RequestParser p(req);
            service_.unlink(p.requireInt("id_conection"),
                            p.requireInt("id_dispositivo"),
                            uid(req));
            return helpers::Response::success();
        }, std::move(cb));
    }

private:
    service::DispositivoService service_;
};

} // namespace app::controllers
