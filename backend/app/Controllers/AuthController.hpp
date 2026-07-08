#pragma once
// ============================================================
//  app/Controllers/AuthController.hpp
//  Rotas de autenticacao. O refresh token vai em cookie
//  HttpOnly (mais seguro que localStorage) e tambem no corpo
//  (para clientes nao-browser). O access token vai no corpo,
//  para o front mandar em Authorization: Bearer.
// ============================================================

#include <drogon/HttpController.h>
#include "BaseController.hpp"
#include "../Service/AuthService.hpp"
#include "../Helpers/RequestParser.hpp"
#include "../Config/AppConfig.hpp"

namespace app::controllers {

class AuthController : public drogon::HttpController<AuthController>,
                      public BaseController {
public:
    METHOD_LIST_BEGIN
    ADD_METHOD_TO(AuthController::register_, "/auth/register", drogon::Post, drogon::Options);
    ADD_METHOD_TO(AuthController::login,     "/auth/login",    drogon::Post, drogon::Options);
    ADD_METHOD_TO(AuthController::refresh,   "/auth/refresh",  drogon::Post, drogon::Options);
    ADD_METHOD_TO(AuthController::logout,    "/auth/logout",   drogon::Post, drogon::Options);
    ADD_METHOD_TO(AuthController::me,        "/auth/me",       drogon::Get,  drogon::Options,
                  "app::middleware::AuthMiddleware");
    METHOD_LIST_END

    void register_(const drogon::HttpRequestPtr& req,
                   std::function<void(const drogon::HttpResponsePtr&)>&& cb) {
        handle([&]() {
            helpers::RequestParser p(req);
            auto u = auth_.register_(p.requireString("nome"),
                                     p.requireString("email"),
                                     p.requireString("senha"));
            return helpers::Response::success(u.toJson(), drogon::k201Created);
        }, std::move(cb));
    }

    void login(const drogon::HttpRequestPtr& req,
               std::function<void(const drogon::HttpResponsePtr&)>&& cb) {
        handle([&]() {
            helpers::RequestParser p(req);
            auto tokens = auth_.login(p.requireString("email"),
                                      p.requireString("senha"));
            return tokenResponse(tokens);
        }, std::move(cb));
    }

    void refresh(const drogon::HttpRequestPtr& req,
                 std::function<void(const drogon::HttpResponsePtr&)>&& cb) {
        handle([&]() {
            std::string rt = readRefreshToken(req);
            auto tokens = auth_.refresh(rt);
            return tokenResponse(tokens);
        }, std::move(cb));
    }

    void logout(const drogon::HttpRequestPtr& req,
                std::function<void(const drogon::HttpResponsePtr&)>&& cb) {
        handle([&]() {
            std::string rt = readRefreshToken(req);
            auth_.logout(rt);
            auto resp = helpers::Response::success();
            expireRefreshCookie(resp);
            return resp;
        }, std::move(cb));
    }

    void me(const drogon::HttpRequestPtr& req,
            std::function<void(const drogon::HttpResponsePtr&)>&& cb) {
        handle([&]() {
            Json::Value j;
            j["id"] = uid(req);
            j["email"] = req->attributes()->get<std::string>("email");
            return helpers::Response::success(j);
        }, std::move(cb));
    }

private:
    // Monta a resposta com access token no corpo e refresh no cookie.
    drogon::HttpResponsePtr tokenResponse(const service::AuthTokens& t) {
        Json::Value data;
        data["access_token"] = t.accessToken;
        data["token_type"] = "Bearer";
        data["expires_in"] = t.accessExpiresIn;
        data["refresh_token"] = t.refreshToken; // para clientes nao-browser
        data["usuario"] = t.usuario.toJson();

        auto resp = helpers::Response::success(data);
        setRefreshCookie(resp, t.refreshToken);
        return resp;
    }

    static std::string readRefreshToken(const drogon::HttpRequestPtr& req) {
        // 1) cookie; 2) corpo JSON (fallback p/ clientes nao-browser).
        std::string c = req->getCookie("refresh_token");
        if (!c.empty()) return c;
        auto json = req->getJsonObject();
        if (json && json->isMember("refresh_token"))
            return (*json)["refresh_token"].asString();
        return "";
    }

    void setRefreshCookie(const drogon::HttpResponsePtr& resp,
                          const std::string& token) {
        drogon::Cookie cookie("refresh_token", token);
        cookie.setHttpOnly(true);
        cookie.setPath("/auth");
        cookie.setMaxAge(config::AppConfig::get().refreshTokenTtl());
        cookie.setSameSite(drogon::Cookie::SameSite::kLax);
        // Em producao com HTTPS: cookie.setSecure(true);
        resp->addCookie(std::move(cookie));
    }

    void expireRefreshCookie(const drogon::HttpResponsePtr& resp) {
        drogon::Cookie cookie("refresh_token", "");
        cookie.setHttpOnly(true);
        cookie.setPath("/auth");
        cookie.setMaxAge(0);
        resp->addCookie(std::move(cookie));
    }

    service::AuthService auth_;
};

} // namespace app::controllers
