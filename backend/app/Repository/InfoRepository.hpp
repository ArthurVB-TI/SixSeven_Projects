#pragma once
// ============================================================
//  app/Repository/InfoRepository.hpp
//  Acesso a 'info' + 'info_historico'. Este e o repositorio
//  que atende o hardware:
//   * push()      -> info_Push (upsert da leitura + historico)
//   * getConfig() -> config que o firmware puxa (E_b, timer)
//   * setConfig() -> site grava E_b/timer p/ o hardware puxar
//   * historico() -> serie temporal p/ graficos
// ============================================================

#include <optional>
#include <vector>
#include "../Config/Database.hpp"
#include "../Models/Info.hpp"
#include "../Models/InfoHistorico.hpp"

namespace app::repository {

class InfoRepository {
public:
    // Chamado pelo endpoint que o hardware usa (POST /data).
    void push(const models::Info& i) {
        auto db = config::Database::client();
        config::Database::callSync(db, 
            "CALL info_Push(?, ?, ?, ?, ?, ?, ?)",
            i.getId(), i.getEb(), i.getEr(), i.getTimer(),
            i.getMer(), i.getVmer(), i.getMvmer());
    }

    // Config que o firmware le (GET /config?id=).
    std::optional<models::Info> getConfig(int id) {
        auto db = config::Database::client();
        auto res = config::Database::callSync(db, "CALL info_Controller('GetConfig', ?, NULL, NULL, NULL, NULL, NULL, NULL)", id);
        if (res.size() == 0) return std::nullopt;
        models::Info info;
        info.setId(res[0]["id"].as<int>());
        info.setEb(res[0]["E_b"].as<int>());
        info.setTimer(res[0]["timer"].as<int>());
        return info;
    }

    // Site grava a config (manipular hardware).
    void setConfig(int id, int E_b, int timer) {
        auto db = config::Database::client();
        config::Database::callSync(db, "CALL info_SetConfig(?, ?, ?)", id, E_b, timer);
    }

    // Ultima leitura completa (pagina de informacoes).
    std::optional<models::Info> findById(int id) {
        auto db = config::Database::client();
        auto res = config::Database::callSync(db, "SELECT * FROM index_info WHERE id = ?", id);
        if (res.size() == 0) return std::nullopt;
        return models::Info::fromRow(res[0]);
    }

    std::vector<models::Info> index() {
        auto db = config::Database::client();
        auto res = config::Database::callSync(db, "CALL info_Controller('Index', NULL, NULL, NULL, NULL, NULL, NULL, NULL)");
        std::vector<models::Info> out;
        for (const auto& row : res) out.push_back(models::Info::fromRow(row));
        return out;
    }

    // Serie temporal (mais recentes primeiro).
    std::vector<models::InfoHistorico> historico(int idConection, int limite) {
        auto db = config::Database::client();
        auto res = config::Database::callSync(db, "CALL info_historico_Index(?, ?)", idConection, limite);
        std::vector<models::InfoHistorico> out;
        for (const auto& row : res) out.push_back(models::InfoHistorico::fromRow(row));
        return out;
    }
};

} // namespace app::repository
