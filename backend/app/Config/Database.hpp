#pragma once
// ============================================================
//  app/Config/Database.hpp
//  Ponto unico de acesso ao banco. Envelopa o DbClient do
//  Drogon (configurado via config/app.config.json -> db_clients).
//  Repositories chamam Database::client() para obter a conexao.
//  Padrao: fachada fina sobre o pool ja gerenciado pelo Drogon.
// ============================================================

#include <drogon/drogon.h>
#include <drogon/orm/DbClient.h>
#include <memory>

namespace app::config {

class Database {
public:
    // Retorna o client "default" declarado no JSON de config.
    static drogon::orm::DbClientPtr client() {
        return drogon::app().getDbClient("default");
    }
};

} // namespace app::config
