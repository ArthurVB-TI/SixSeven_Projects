#pragma once
// ============================================================
//  app/Repository/DispositivoRepository.hpp
//  Acesso a dados de 'dispositivos' via dispositivos_Controller.
// ============================================================

#include <vector>
#include "../Config/Database.hpp"
#include "../Models/Dispositivo.hpp"

namespace app::repository {

class DispositivoRepository {
public:
    int create(const models::Dispositivo& d) {
        auto db = config::Database::client();
        auto res = config::Database::callSync(db, 
            "CALL dispositivos_Controller('Create', NULL, ?, ?, ?)",
            d.getNome(), d.getTipo(), d.getIdUsuario());
        if (res.size() > 0 && !res[0]["id"].isNull())
            return res[0]["id"].as<int>();
        return 0;
    }

    void update(const models::Dispositivo& d) {
        auto db = config::Database::client();
        config::Database::callSync(db, 
            "CALL dispositivos_Controller('Update', ?, ?, ?, ?)",
            d.getId(), d.getNome(), d.getTipo(), d.getIdUsuario());
    }

    void remove(int id) {
        auto db = config::Database::client();
        config::Database::callSync(db, "CALL dispositivos_Controller('Delete', ?, NULL, NULL, NULL)", id);
    }

    std::vector<models::Dispositivo> index() {
        auto db = config::Database::client();
        auto res = config::Database::callSync(db, "CALL dispositivos_Controller('Index', NULL, NULL, NULL, NULL)");
        std::vector<models::Dispositivo> out;
        for (const auto& row : res) out.push_back(models::Dispositivo::fromRow(row));
        return out;
    }
};

} // namespace app::repository
