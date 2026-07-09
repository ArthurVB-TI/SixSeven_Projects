#pragma once
// ============================================================
//  app/Repository/RefreshTokenRepository.hpp
//  Persistencia da sessao (refresh tokens) via
//  refresh_token_Controller. Guarda somente o hash do token.
// ============================================================

#include <optional>
#include <string>
#include "../Config/Database.hpp"

namespace app::repository {

struct RefreshTokenRow {
    int64_t id{0};
    int idUsuario{0};
    std::string tokenHash;
    std::string expiraEm;
    bool revogado{false};
};

class RefreshTokenRepository {
public:
    // expiraEm no formato 'YYYY-MM-DD HH:MM:SS'.
    void create(int idUsuario, const std::string& tokenHash, const std::string& expiraEm) {
        auto db = config::Database::client();
        config::Database::callSync(db, "CALL refresh_token_Controller('Create', ?, ?, ?)",
                        idUsuario, tokenHash, expiraEm);
    }

    // Retorna a linha somente se valida (nao revogada e nao expirada).
    std::optional<RefreshTokenRow> findValid(const std::string& tokenHash) {
        auto db = config::Database::client();
        auto res = config::Database::callSync(db, "CALL refresh_token_Controller('Find', NULL, ?, NULL)", tokenHash);
        if (res.size() == 0) return std::nullopt;
        RefreshTokenRow r;
        r.id = res[0]["id"].as<int64_t>();
        r.idUsuario = res[0]["id_usuario"].as<int>();
        r.tokenHash = res[0]["token_hash"].as<std::string>();
        r.expiraEm = res[0]["expira_em"].as<std::string>();
        r.revogado = res[0]["revogado"].as<int>() != 0;
        return r;
    }

    void revoke(const std::string& tokenHash) {
        auto db = config::Database::client();
        config::Database::callSync(db, "CALL refresh_token_Controller('Revoke', NULL, ?, NULL)", tokenHash);
    }

    void revokeAll(int idUsuario) {
        auto db = config::Database::client();
        config::Database::callSync(db, "CALL refresh_token_Controller('RevokeAll', ?, NULL, NULL)", idUsuario);
    }
};

} // namespace app::repository
