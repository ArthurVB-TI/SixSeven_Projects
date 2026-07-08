#pragma once
// ============================================================
//  app/Config/AppConfig.hpp
//  Config DENTRO da pasta app: expoe, de forma tipada, os
//  parametros de negocio/seguranca que o codigo consome.
//  Le do arquivo carregado pelo Drogon (config/app.config.json)
//  atraves de app().getCustomConfig().
//  Padrao: Singleton (uma unica fonte de verdade em runtime).
// ============================================================

#include <drogon/drogon.h>
#include <string>
#include <cstdint>

namespace app::config {

class AppConfig {
public:
    static const AppConfig& get() {
        static AppConfig instance;
        return instance;
    }

    const std::string& jwtSecret()        const { return jwtSecret_; }
    int  accessTokenTtl()                 const { return accessTtl_; }
    int  refreshTokenTtl()                const { return refreshTtl_; }
    int  bcryptCost()                     const { return bcryptCost_; }
    const std::string& corsAllowOrigin()  const { return corsOrigin_; }
    const std::string& appEnv()           const { return appEnv_; }

    AppConfig(const AppConfig&) = delete;
    AppConfig& operator=(const AppConfig&) = delete;

private:
    AppConfig() { load(); }

    void load() {
        const auto& custom = drogon::app().getCustomConfig();
        appEnv_ = custom.get("app_env", "development").asString();

        const auto& sec = custom["security"];
        jwtSecret_  = sec.get("jwt_secret", "changeme").asString();
        accessTtl_  = sec.get("access_token_ttl_seconds", 900).asInt();
        refreshTtl_ = sec.get("refresh_token_ttl_seconds", 2592000).asInt();
        bcryptCost_ = sec.get("bcrypt_cost", 10).asInt();
        corsOrigin_ = sec.get("cors_allow_origin", "*").asString();
    }

    std::string jwtSecret_;
    std::string corsOrigin_;
    std::string appEnv_;
    int accessTtl_{900};
    int refreshTtl_{2592000};
    int bcryptCost_{10};
};

} // namespace app::config
