#pragma once
// ============================================================
//  app/Repository/ConectionRepository.hpp
//  Acesso a dados de 'conections' via conections_Controller.
// ============================================================

#include <optional>
#include <vector>
#include "../Config/Database.hpp"
#include "../Models/Conection.hpp"

namespace app::repository {

class ConectionRepository {
public:
    int create(const models::Conection& c) {
        auto db = config::Database::client();
        auto res = db->execSqlSync(
            "CALL conections_Controller('Create', NULL, ?, ?, ?, ?)",
            c.getNome(), c.getCanActive(), c.getIsActive(), c.getIdUsuario());
        if (res.size() > 0 && !res[0]["id"].isNull())
            return res[0]["id"].as<int>();
        return 0;
    }

    void update(const models::Conection& c) {
        auto db = config::Database::client();
        db->execSqlSync(
            "CALL conections_Controller('Update', ?, ?, ?, ?, ?)",
            c.getId(), c.getNome(), c.getCanActive(), c.getIsActive(), c.getIdUsuario());
    }

    void remove(int id) {
        auto db = config::Database::client();
        db->execSqlSync("CALL conections_Controller('Delete', ?, NULL, NULL, NULL, NULL)", id);
    }

    std::vector<models::Conection> index() {
        auto db = config::Database::client();
        auto res = db->execSqlSync("CALL conections_Controller('Index', NULL, NULL, NULL, NULL, NULL)");
        std::vector<models::Conection> out;
        for (const auto& row : res) out.push_back(models::Conection::fromRow(row));
        return out;
    }

    std::vector<models::Conection> indexByUsuario(int idUsuario) {
        auto db = config::Database::client();
        auto res = db->execSqlSync(
            "CALL conections_Controller('IndexByUsuario', NULL, NULL, NULL, NULL, ?)", idUsuario);
        std::vector<models::Conection> out;
        for (const auto& row : res) out.push_back(models::Conection::fromRow(row));
        return out;
    }

    // Busca direta (para checar posse antes de operar).
    std::optional<models::Conection> findById(int id) {
        auto db = config::Database::client();
        auto res = db->execSqlSync("SELECT * FROM index_conections WHERE id = ?", id);
        if (res.size() == 0) return std::nullopt;
        return models::Conection::fromRow(res[0]);
    }
};

} // namespace app::repository
