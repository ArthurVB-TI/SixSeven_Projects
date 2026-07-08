#pragma once
// ============================================================
//  app/Middleware/AuthMiddleware.hpp
//  Filtro de autenticacao para rotas protegidas. Le o header
//  Authorization: Bearer <access_token>, valida a assinatura e
//  a expiracao (Jwt) e injeta o id do usuario nos atributos da
//  requisicao ("uid"), disponivel para os controllers.
//  Falha => 401 padronizado.
// ============================================================

#include <drogon/HttpFilter.h>
#include "../Security/Jwt.hpp"
#include "../Config/AppConfig.hpp"
#include "../Helpers/Response.hpp"

namespace app::middleware {

class AuthMiddleware : public drogon::HttpFilter<AuthMiddleware> {
public:
    void doFilter(const drogon::HttpRequestPtr& req,
                  drogon::FilterCallback&& fcb,
                  drogon::FilterChainCallback&& fccb) override {
        std::string auth = req->getHeader("Authorization");
        const std::string prefix = "Bearer ";

        if (auth.size() <= prefix.size() ||
            auth.compare(0, prefix.size(), prefix) != 0) {
            fcb(helpers::Response::error("Token de acesso ausente",
                                         drogon::k401Unauthorized));
            return;
        }

        std::string token = auth.substr(prefix.size());
        auto claims = security::Jwt::decode(token,
                        config::AppConfig::get().jwtSecret());

        if (!claims.has_value()) {
            fcb(helpers::Response::error("Token invalido ou expirado",
                                         drogon::k401Unauthorized));
            return;
        }

        // Injeta o uid para os controllers lerem via getAttributes().
        req->attributes()->insert("uid", static_cast<int>(claims->sub));
        req->attributes()->insert("email", claims->email);
        fccb();
    }
};

} // namespace app::middleware
