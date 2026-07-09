#pragma once
// ============================================================
//  app/Repository/DispositivoConectionRepository.hpp
//  Relacao N:N via dispositivos_conections_Controller.
// ============================================================

#include <vector>
#include "../Config/Database.hpp"
#include "../Models/DispositivoConection.hpp"

namespace app::repository {

class DispositivoConectionRepository {
public:
    void create(int idConections, int idDispositivos) {
        auto db = config::Database::client();
        config::Database::callSync(db, 
            "CALL dispositivos_conections_Controller('Create', ?, ?)",
            idConections, idDispositivos);
    }

    void remove(int idConections, int idDispositivos) {
        auto db = config::Database::client();
        config::Database::callSync(db, 
            "CALL dispositivos_conections_Controller('Delete', ?, ?)",
            idConections, idDispositivos);
    }

    std::vector<models::DispositivoConection> index() {
        auto db = config::Database::client();
        auto res = config::Database::callSync(db, "CALL dispositivos_conections_Controller('Index', NULL, NULL)");
        std::vector<models::DispositivoConection> out;
        for (const auto& row : res) out.push_back(models::DispositivoConection::fromRow(row));
        return out;
    }
};

} // namespace app::repository
