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
#include <atomic>
#include <future>
#include <memory>
#include <stdexcept>

namespace app::config {

class Database {
public:
    // Retorna o client "default" declarado no JSON de config.
    static drogon::orm::DbClientPtr client() {
        return drogon::app().getDbClient("default");
    }

    // Versao sincrona segura para stored procedures. Um "CALL ..."
    // no MySQL devolve MULTIPLOS result sets (o resultado + o OK
    // final) e o Drogon invoca o callback uma vez por result set;
    // o execSqlSync nativo usa uma std::promise que so aceita um
    // set_value e aborta o processo na segunda chamada. Aqui
    // guardamos o primeiro result set e ignoramos os demais.
    template <typename... Args>
    static drogon::orm::Result callSync(const drogon::orm::DbClientPtr& db,
                                        const std::string& sql,
                                        Args&&... args) {
        auto pro = std::make_shared<std::promise<drogon::orm::Result>>();
        auto done = std::make_shared<std::atomic_bool>(false);
        auto fut = pro->get_future();
        db->execSqlAsync(
            sql,
            [pro, done](const drogon::orm::Result& r) {
                if (!done->exchange(true)) pro->set_value(r);
            },
            [pro, done](const drogon::orm::DrogonDbException& e) {
                if (!done->exchange(true))
                    pro->set_exception(std::make_exception_ptr(
                        std::runtime_error(e.base().what())));
            },
            std::forward<Args>(args)...);
        return fut.get();
    }
};

} // namespace app::config
