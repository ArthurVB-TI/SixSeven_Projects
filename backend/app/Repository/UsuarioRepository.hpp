#pragma once
// ============================================================
//  app/Repository/UsuarioRepository.hpp
//  Acesso a dados de 'usuario' via procedures (usuario_Controller
//  e usuario_FindByEmail). Toda query usa prepared statements do
//  Drogon (protecao contra SQL injection). Retorna Models.
// ============================================================

#include <optional>
#include <vector>
#include <drogon/orm/Mapper.h>
#include "../Config/Database.hpp"
#include "../Models/Usuario.hpp"

namespace app::repository {

class UsuarioRepository {
public:
    // Cria e retorna o id gerado (0 se a procedure recusou).
    int create(const std::string& nome, const std::string& email,
               const std::string& senhaHash) {
        auto db = config::Database::client();
        auto res = db->execSqlSync(
            "CALL usuario_Controller('Create', NULL, ?, ?, ?)",
            nome, email, senhaHash);
        if (res.size() > 0 && !res[0]["id"].isNull())
            return res[0]["id"].as<int>();
        return 0;
    }

    void update(const models::Usuario& u) {
        auto db = config::Database::client();
        db->execSqlSync("CALL usuario_Controller('Update', ?, ?, ?, ?)",
                        u.getId(), u.getNome(), u.getEmail(), u.getSenha());
    }

    void remove(int id) {
        auto db = config::Database::client();
        db->execSqlSync("CALL usuario_Controller('Delete', ?, NULL, NULL, NULL)", id);
    }

    std::vector<models::Usuario> index() {
        auto db = config::Database::client();
        auto res = db->execSqlSync("CALL usuario_Controller('Index', NULL, NULL, NULL, NULL)");
        std::vector<models::Usuario> out;
        out.reserve(res.size());
        for (const auto& row : res) out.push_back(models::Usuario::fromRow(row));
        return out;
    }

    // Usado no login: traz o hash da senha.
    std::optional<models::Usuario> findByEmail(const std::string& email) {
        auto db = config::Database::client();
        auto res = db->execSqlSync(
            "CALL usuario_Controller('FindByEmail', NULL, NULL, ?, NULL)", email);
        if (res.size() == 0) return std::nullopt;
        return models::Usuario::fromRow(res[0]);
    }
};

} // namespace app::repository
